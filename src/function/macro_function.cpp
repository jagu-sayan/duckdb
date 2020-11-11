#include "duckdb/function/macro_function.hpp"

#include "duckdb/catalog/catalog_entry/macro_function_catalog_entry.hpp"
#include "duckdb/planner/expression_iterator.hpp"

namespace duckdb {

MacroFunction::MacroFunction(unique_ptr<ParsedExpression> expression) : expression(move(expression)) {
}

// for nested function expressions
static ParsedExpression &GetParsedExpressionRecursive(ParsedExpression &expr) {
	if (expr.GetExpressionClass() != ExpressionClass::BOUND_EXPRESSION)
		return expr;
	auto &bound_expr = (BoundExpression &)expr;
	return GetParsedExpressionRecursive(*bound_expr.parsed_expr);
}

unique_ptr<Expression> MacroFunction::BindMacroFunction(Binder &binder, ExpressionBinder &expr_binder,
                                                        MacroFunctionCatalogEntry &function,
                                                        vector<unique_ptr<Expression>> arguments, string &error) {
	// create macro_binder in binder to bind parameters to arguments
	auto &macro_func = function.function;
	auto &parameters = macro_func->parameters;
	if (parameters.size() != arguments.size()) {
		error = StringUtil::Format("Macro function '%s(%s)' requires ", function.name,
		                           StringUtil::Join(parameters, parameters.size(), ", ",
		                                            [](const unique_ptr<ParsedExpression> &p) { return ((ColumnRefExpression &)*p).column_name; }));
		error += parameters.size() == 1 ? "a single argument" : StringUtil::Format("%i arguments", parameters.size());
		error += ", but ";
		error += arguments.size() == 1 ? "a single argument was" : StringUtil::Format("%i arguments were", arguments.size());
		error += " provided.";
		return nullptr;
	}
	vector<LogicalType> types;
	vector<string> names;
	for (idx_t i = 0; i < parameters.size(); i++) {
		types.push_back(arguments[i]->return_type);
		auto &param = (ColumnRefExpression &)*parameters[i];
		names.push_back(param.column_name);
	}
	binder.macro_binding = make_shared<MacroBinding>(types, names);
	binder.macro_binding->arguments = move(arguments);

	// now we perform the binding
	auto parsed_expression = macro_func->expression->Copy();
	auto result = expr_binder.Bind(parsed_expression);

	// delete the macro binding so that it cannot effect bindings outside of the macro call
	binder.macro_binding.reset();

	return result;
}

} // namespace duckdb
