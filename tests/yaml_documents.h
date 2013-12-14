const char *yaml_good =
"---\n"
"core:\n"
"    address : \"127.0.0.1\"\n"
"    port: 8080\n"
"    test_list:\n"
"        - first\n"
"        - second\n"
"        - third\n"
"    test_bool: yes\n"
"    test_list_inline: [1,2,3]\n"
"skipped:\n"
"    this: \"is skipped\"\n"
"    with_all: \"options\"\n";

const char *yaml_good_inline =
"---\n"
"core: {address: \"127.0.0.1\", port: 8080, test_list: [\"first\", \"second\", \"third\"], test_bool: on, test_list_inline: [1,2,3]}";

const char *yaml_no_stream = "";
const char *yaml_empty_document = "---\n";
const char *yaml_document_no_mapping = "---\n[1,2,3]";
const char *yaml_no_start_document = "core: {address: \"0.0.0.0\"}";
const char *yaml_wrong_int = "---\ncore: {intvalue: absderrg}";
const char *yaml_wrong_int2 = "---\ncore: {intvalue: 100abc}";
const char *yaml_wrong_bool = "---\ncore: {boolvalue: absderrg}";
const char *yaml_wrong_string = "---\ncore: {strvalue: }";
const char *yaml_wrong_multivalued = "---\ncore: {singlevalue: [1,2,3]}";
