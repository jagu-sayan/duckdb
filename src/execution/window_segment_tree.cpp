#include "duckdb/execution/window_segment_tree.hpp"

#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/common/algorithm.hpp"
#include "duckdb/common/helper.hpp"

#include <utility>

namespace duckdb {

//===--------------------------------------------------------------------===//
// WindowAggregateState
//===--------------------------------------------------------------------===//

WindowAggregateState::WindowAggregateState(AggregateObject aggr, const LogicalType &result_type_p,
                                           idx_t partition_count_p)
    : aggr(std::move(aggr)), result_type(result_type_p), partition_count(partition_count_p),
      state(aggr.function.state_size()), statef(Value::POINTER(CastPointerToValue(state.data()))), filter_count(0) {
	statef.SetVectorType(VectorType::FLAT_VECTOR); // Prevent conversion of results to constants
}

WindowAggregateState::~WindowAggregateState() {
}

void WindowAggregateState::AggregateInit() {
	aggr.function.initialize(state.data());
}

void WindowAggregateState::AggegateFinal(Vector &result, idx_t rid) {
	AggregateInputData aggr_input_data(aggr.GetFunctionData(), Allocator::DefaultAllocator());
	aggr.function.finalize(statef, aggr_input_data, result, 1, rid);

	if (aggr.function.destructor) {
		aggr.function.destructor(statef, aggr_input_data, 1);
	}
}

void WindowAggregateState::Sink(DataChunk &payload_chunk, SelectionVector *filter_sel, idx_t filtered) {
	if (!inputs.ColumnCount() && payload_chunk.ColumnCount()) {
		inputs.Initialize(Allocator::DefaultAllocator(), payload_chunk.GetTypes());
	}
	if (inputs.ColumnCount()) {
		inputs.Append(payload_chunk, true);
	}
	if (filter_sel) {
		//	Lazy instantiation
		if (!filter_mask.IsMaskSet()) {
			// 	Start with all invalid and set the ones that pass
			filter_bits.resize(ValidityMask::ValidityMaskSize(partition_count), 0);
			filter_mask.Initialize(filter_bits.data());
		}
		for (idx_t f = 0; f < filtered; ++f) {
			filter_mask.SetValid(filter_count + filter_sel->get_index(f));
		}
		filter_count += filtered;
	}
}

void WindowAggregateState::Finalize() {
}

void WindowAggregateState::Compute(Vector &result, idx_t rid, idx_t start, idx_t end) {
}

//===--------------------------------------------------------------------===//
// WindowConstantAggregate
//===--------------------------------------------------------------------===//

WindowConstantAggregate::WindowConstantAggregate(AggregateObject aggr, const LogicalType &result_type,
                                                 const ValidityMask &partition_mask, const idx_t count)
    : WindowAggregateState(std::move(aggr), result_type, count), partition(0), row(0),
      statep(Value::POINTER(CastPointerToValue(state.data()))) {

	// Locate the partition boundaries
	idx_t start = 0;
	if (partition_mask.AllValid()) {
		partition_offsets.emplace_back(0);
	} else {
		idx_t entry_idx;
		idx_t shift;
		while (start < count) {
			partition_mask.GetEntryIndex(start, entry_idx, shift);

			//	If start is aligned with the start of a block,
			//	and the block is blank, then skip forward one block.
			const auto block = partition_mask.GetValidityEntry(entry_idx);
			if (partition_mask.NoneValid(block) && !shift) {
				start += ValidityMask::BITS_PER_VALUE;
				continue;
			}

			// Loop over the block
			for (; shift < ValidityMask::BITS_PER_VALUE && start < count; ++shift, ++start) {
				if (partition_mask.RowIsValid(block, shift)) {
					partition_offsets.emplace_back(start);
				}
			}
		}
	}

	//	Initialise the vector for caching the results
	results = make_uniq<Vector>(result_type, partition_offsets.size());
	partition_offsets.emplace_back(count);

	//	Start the first aggregate
	AggregateInit();
}

void WindowConstantAggregate::Sink(DataChunk &payload_chunk, SelectionVector *filter_sel, idx_t filtered) {
	const auto chunk_begin = row;
	const auto chunk_end = chunk_begin + payload_chunk.size();

	if (!inputs.ColumnCount() && payload_chunk.ColumnCount()) {
		inputs.Initialize(Allocator::DefaultAllocator(), payload_chunk.GetTypes());
	}

	AggregateInputData aggr_input_data(aggr.GetFunctionData(), Allocator::DefaultAllocator());
	idx_t begin = 0;
	idx_t filter_idx = 0;
	auto partition_end = partition_offsets[partition + 1];
	while (row < chunk_end) {
		if (row == partition_end) {
			AggegateFinal(*results, partition++);
			AggregateInit();
			partition_end = partition_offsets[partition + 1];
		}
		partition_end = MinValue(partition_end, chunk_end);
		auto end = partition_end - chunk_begin;

		inputs.Reset();
		if (filter_sel) {
			// 	Slice to any filtered rows in [begin, end)
			SelectionVector sel;

			//	Find the first value in [begin, end)
			for (; filter_idx < filtered; ++filter_idx) {
				auto idx = filter_sel->get_index(filter_idx);
				if (idx >= begin) {
					break;
				}
			}

			//	Find the first value in [end, filtered)
			sel.Initialize(filter_sel->data() + filter_idx);
			idx_t nsel = 0;
			for (; filter_idx < filtered; ++filter_idx, ++nsel) {
				auto idx = filter_sel->get_index(filter_idx);
				if (idx >= end) {
					break;
				}
			}

			if (nsel != inputs.size()) {
				inputs.Slice(payload_chunk, sel, nsel);
			}
		} else {
			//	Slice to [begin, end)
			if (begin) {
				for (idx_t c = 0; c < payload_chunk.ColumnCount(); ++c) {
					inputs.data[c].Slice(payload_chunk.data[c], begin, end);
				}
			} else {
				inputs.Reference(payload_chunk);
			}
			inputs.SetCardinality(end - begin);
		}

		//	Aggregate the filtered rows into a single state
		const auto count = inputs.size();
		if (aggr.function.simple_update) {
			aggr.function.simple_update(inputs.data.data(), aggr_input_data, inputs.ColumnCount(), state.data(), count);
		} else {
			aggr.function.update(inputs.data.data(), aggr_input_data, inputs.ColumnCount(), statep, count);
		}

		//	Skip filtered rows too!
		row += end - begin;
		begin = end;
	}
}

void WindowConstantAggregate::Finalize() {
	AggegateFinal(*results, partition++);

	partition = 0;
	row = 0;
}

void WindowConstantAggregate::Compute(Vector &target, idx_t rid, idx_t start, idx_t end) {
	//	Find the partition containing [start, end)
	while (start < partition_offsets[partition] || partition_offsets[partition + 1] <= start) {
		++partition;
	}
	D_ASSERT(partition_offsets[partition] <= start);
	D_ASSERT(partition + 1 < partition_offsets.size());
	D_ASSERT(end <= partition_offsets[partition + 1]);

	// Copy the value
	VectorOperations::Copy(*results, target, partition + 1, partition, rid);
}

//===--------------------------------------------------------------------===//
// WindowCustomAggregate
//===--------------------------------------------------------------------===//
WindowCustomAggregate::WindowCustomAggregate(AggregateObject aggr, const LogicalType &result_type, idx_t count)
    : WindowAggregateState(std::move(aggr), result_type, count) {
	// if we have a frame-by-frame method, share the single state
	AggregateInit();
}

WindowCustomAggregate::~WindowCustomAggregate() {
	if (aggr.function.destructor) {
		AggregateInputData aggr_input_data(aggr.GetFunctionData(), Allocator::DefaultAllocator());
		aggr.function.destructor(statef, aggr_input_data, 1);
	}
}

void WindowCustomAggregate::Compute(Vector &result, idx_t rid, idx_t begin, idx_t end) {
	// Frame boundaries
	auto prev = frame;
	frame = FrameBounds(begin, end);

	// Extract the range
	AggregateInputData aggr_input_data(aggr.GetFunctionData(), Allocator::DefaultAllocator());
	aggr.function.window(inputs.data.data(), filter_mask, aggr_input_data, inputs.ColumnCount(), state.data(), frame,
	                     prev, result, rid, 0);
}

//===--------------------------------------------------------------------===//
// WindowSegmentTree
//===--------------------------------------------------------------------===//
WindowSegmentTree::WindowSegmentTree(AggregateObject aggr, const LogicalType &result_type, idx_t count,
                                     WindowAggregationMode mode_p)
    : WindowAggregateState(std::move(aggr), result_type, count),
      statep(Value::POINTER(CastPointerToValue(state.data()))), frame(0, 0), statel(LogicalType::POINTER),
      flush_count(0), internal_nodes(0), mode(mode_p) {
	statep.Flatten(STANDARD_VECTOR_SIZE);
	statef.SetVectorType(VectorType::FLAT_VECTOR); // Prevent conversion of results to constants
}

void WindowSegmentTree::Finalize() {
	if (inputs.ColumnCount() > 0) {
		leaves.Initialize(Allocator::DefaultAllocator(), inputs.GetTypes());
		leaves.Reference(inputs);

		//	In order to share the SV, we can't have any leaves that are already dictionaries.
		for (column_t i = 0; i < leaves.ColumnCount(); i++) {
			auto &leaf = leaves.data[i];
			switch (leaf.GetVectorType()) {
			case VectorType::DICTIONARY_VECTOR:
			case VectorType::FSST_VECTOR:
				leaf.Flatten(leaves.size());
			default:
				break;
			}
		}
		//	The inputs share an SV so we can quickly pick out values
		filter_sel.Initialize();
		//	What we slice to is not important now - we just want the SV to be shared.
		leaves.Slice(filter_sel, STANDARD_VECTOR_SIZE);
		if (aggr.function.combine && UseCombineAPI()) {
			ConstructTree();
		}
	}
}

WindowSegmentTree::~WindowSegmentTree() {
	if (!aggr.function.destructor) {
		// nothing to destroy
		return;
	}
	AggregateInputData aggr_input_data(aggr.GetFunctionData(), Allocator::DefaultAllocator());
	// call the destructor for all the intermediate states
	data_ptr_t address_data[STANDARD_VECTOR_SIZE];
	Vector addresses(LogicalType::POINTER, data_ptr_cast(address_data));
	idx_t count = 0;
	for (idx_t i = 0; i < internal_nodes; i++) {
		address_data[count++] = data_ptr_t(levels_flat_native.get() + i * state.size());
		if (count == STANDARD_VECTOR_SIZE) {
			aggr.function.destructor(addresses, aggr_input_data, count);
			count = 0;
		}
	}
	if (count > 0) {
		aggr.function.destructor(addresses, aggr_input_data, count);
	}
}

void WindowSegmentTree::FlushStates(idx_t l_idx) {
	if (!flush_count) {
		return;
	}

	AggregateInputData aggr_input_data(aggr.GetFunctionData(), Allocator::DefaultAllocator());
	if (l_idx) {
		statel.Verify(flush_count);
		aggr.function.combine(statel, statep, aggr_input_data, flush_count);
	} else {
		leaves.SetCardinality(flush_count);
		aggr.function.update(&leaves.data[0], aggr_input_data, leaves.ColumnCount(), statep, flush_count);

		//	Some update functions mangle our dictionaries.
		//	so we have to check and unmangle them...
		for (column_t i = 0; i < leaves.ColumnCount(); i++) {
			auto &leaf = leaves.data[i];
			if (leaf.GetVectorType() != VectorType::DICTIONARY_VECTOR ||
			    DictionaryVector::SelVector(leaf).data() != filter_sel.data()) {
				leaf.Slice(inputs.data[i], filter_sel, STANDARD_VECTOR_SIZE);
			}
		}
	}

	flush_count = 0;
}

void WindowSegmentTree::ExtractFrame(idx_t begin, idx_t end) {
	const auto count = end - begin;
	D_ASSERT(count <= TREE_FANOUT);

	//	If we are not filtering,
	//	just update the shared dictionary selection to the range
	//	Otherwise set it to the input rows that pass the filter
	if (filter_mask.AllValid()) {
		for (idx_t i = 0; i < count; ++i) {
			filter_sel.set_index(flush_count++, begin + i);
			if (flush_count >= STANDARD_VECTOR_SIZE) {
				FlushStates(0);
			}
		}
	} else {
		for (idx_t i = begin; i < end; ++i) {
			if (filter_mask.RowIsValid(i)) {
				filter_sel.set_index(flush_count++, i);
				if (flush_count >= STANDARD_VECTOR_SIZE) {
					FlushStates(0);
				}
			}
		}
	}
}

void WindowSegmentTree::WindowSegmentValue(idx_t l_idx, idx_t begin, idx_t end) {
	D_ASSERT(begin <= end);
	if (begin == end || inputs.ColumnCount() == 0) {
		return;
	}

	const auto count = end - begin;
	if (l_idx == 0) {
		ExtractFrame(begin, end);
	} else {
		// find out where the states begin
		data_ptr_t begin_ptr = levels_flat_native.get() + state.size() * (begin + levels_flat_start[l_idx - 1]);
		// set up a vector of pointers that point towards the set of states
		auto pdata = FlatVector::GetData<data_ptr_t>(statel);
		for (idx_t i = 0; i < count; i++) {
			pdata[flush_count++] = begin_ptr;
			begin_ptr += state.size();
			if (flush_count >= STANDARD_VECTOR_SIZE) {
				FlushStates(l_idx);
			}
		}
	}
}

void WindowSegmentTree::ConstructTree() {
	D_ASSERT(inputs.ColumnCount() > 0);

	// compute space required to store internal nodes of segment tree
	internal_nodes = 0;
	idx_t level_nodes = inputs.size();
	do {
		level_nodes = (level_nodes + (TREE_FANOUT - 1)) / TREE_FANOUT;
		internal_nodes += level_nodes;
	} while (level_nodes > 1);
	levels_flat_native = make_unsafe_uniq_array<data_t>(internal_nodes * state.size());
	levels_flat_start.push_back(0);

	idx_t levels_flat_offset = 0;
	idx_t level_current = 0;
	// level 0 is data itself
	idx_t level_size;
	// iterate over the levels of the segment tree
	while ((level_size =
	            (level_current == 0 ? inputs.size() : levels_flat_offset - levels_flat_start[level_current - 1])) > 1) {
		for (idx_t pos = 0; pos < level_size; pos += TREE_FANOUT) {
			// compute the aggregate for this entry in the segment tree
			AggregateInit();
			WindowSegmentValue(level_current, pos, MinValue(level_size, pos + TREE_FANOUT));
			FlushStates(level_current);

			memcpy(levels_flat_native.get() + (levels_flat_offset * state.size()), state.data(), state.size());

			levels_flat_offset++;
		}

		levels_flat_start.push_back(levels_flat_offset);
		level_current++;
	}

	// Corner case: single element in the window
	if (levels_flat_offset == 0) {
		aggr.function.initialize(levels_flat_native.get());
	}
}

void WindowSegmentTree::Compute(Vector &result, idx_t rid, idx_t begin, idx_t end) {
	AggregateInit();
	idx_t l_idx = 0;

	// Aggregate everything at once if we can't combine states
	if (!aggr.function.combine || !UseCombineAPI()) {
		WindowSegmentValue(l_idx, begin, end);
		FlushStates(l_idx);
		AggegateFinal(result, rid);
		return;
	}

	for (; l_idx < levels_flat_start.size() + 1; l_idx++) {
		idx_t parent_begin = begin / TREE_FANOUT;
		idx_t parent_end = end / TREE_FANOUT;
		if (parent_begin == parent_end) {
			WindowSegmentValue(l_idx, begin, end);
			break;
		}
		idx_t group_begin = parent_begin * TREE_FANOUT;
		if (begin != group_begin) {
			WindowSegmentValue(l_idx, begin, group_begin + TREE_FANOUT);
			parent_begin++;
		}
		idx_t group_end = parent_end * TREE_FANOUT;
		if (end != group_end) {
			WindowSegmentValue(l_idx, group_end, end);
		}
		begin = parent_begin;
		end = parent_end;

		//	Flush leaf values separately.
		if (!l_idx) {
			FlushStates(l_idx);
		}
	}

	FlushStates(l_idx);

	AggegateFinal(result, rid);
}

} // namespace duckdb
