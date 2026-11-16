extends TestSuite

func _init() -> void:
	icon = "❌"

func test_invalid_identation():
	var invalid_yaml := """
key: value
  indentation: wrong
"""
	var result := YAML.parse(invalid_yaml)
	expect_equal(result.get_error(), "multiline scalars cannot be used as keys (line 3, column 14)", "Expected parse error")

func test_unmatched_quotes():
	var unmatched_quotes := """
message: "This quote is not closed
next_line: value
"""
	var result = YAML.parse(unmatched_quotes)
	expect_equal(result.get_error(), "bad indentation (line 3, column 1)", "Expected parse error")

func test_circular_reference():
	var dict1 = {}
	var dict2 = {"ref": dict1}
	dict1["circular"] = dict2

	var result := YAML.stringify(dict1)
	expect_equal(result.get_error(), "Maximum nesting depth exceeded (100). Possible circular reference?", "Expected circular reference")

func test_syntax_validation():
	var invalid_yaml := """
key: value
- invalid
  list
  format
"""
	var result = YAML.validate_syntax(invalid_yaml)
	expect_equal(result.get_error(), "parse error (line 3, column 1)", "Expected parse error")

func test_error_details():
	var yaml_with_error := """
valid_line: value
- invalid line: value
another_line: value
"""

	var result := YAML.parse(yaml_with_error)
	expect_equal(result.get_error(), "parse error (line 3, column 1)", "Expected parse error")
