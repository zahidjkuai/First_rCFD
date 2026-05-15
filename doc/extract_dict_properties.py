import re

# Define a set of variable names to exclude
# exact matches
blacklist = {
    "max_phase_data_count",
    "mode",
    "number_of_concentration_data",
    "d2_diam",
    "d3_diam",
    "user",
    "adapt_vof_stitching_on",
    "number_of_adapt_vof_loops",
    "number_of_cells",
    "number_of_faces",
    "number_of_int_node_references",
    "number_of_ext_node_references",
    "probe_count",
    "probe_layer",
    "probe_cell",
    "probe_type",
    "probe_phase",
    "probe_data",
    "probe_value",
    "probe_x"
}
# endswith matches
blacklist_suffixes = {"_Dict"}


type_aliases = {
    "char*": "string",
    "short": "integer",
    "short*": "integer",
    "int": "integer",
    "double": "real",
    "double*": "real"
}


# Define a set of variable names that have integer and string type variants
variants = {
    "report_priority",
    "type",
    "format"
}


# Define a set of variable names that cannot be extracted from source code
extra_dict_props = {
    'Dump_Dict': [
        {
            'name': 'probe_cell_data',
            'type': 'vector',
            'comment': 'add a probe location for phase-specific cell data (user-defined function only)',
            'default': 'none'
        },
        {
            'name': "probe_vof",
            'type': "vector",
            'comment': "add a probe location for phase-specific vof (user-defined function only)",
            'default': 'none'
        },
        {
            'name': "probe_user",
            'type': "vector",
            'comment': "add a probe location for user-defined data (user-defined function only)",
            'default': 'none'
        },
        {
            'name': "probe_rec_user",
            'type': "vector",
            'comment': "add a probe location for recurrence-specific user-defined data (user-defined function only)",
            'default': 'none'
        },
        {
            'name': "probe_type",
            'type': "string",
            'comment': "define probe type (`\"cell\"`, `\"vof\"`, `\"user\"` or `\"rec_user\"`) (TOML file only)",
            'default': 'none'
        },
        {
            'name': "probe_data",
            'type': "int",
            'comment': "define probe data index (for all probe types except `\"vof\"`) (TOML file only)",
            'default': 'none'
        },
        {
            'name': "probe_position",
            'type': "vector",
            'comment': "define probe position (TOML file only)",
            'default': 'none'
        }
    ]
}


def remove_header_comments(code):
    """
    Remove block (/*** */) comments,
    """
    # First remove block comments that are NOT doxygen trailing comments
    code = re.sub(r'/\*\*\*.*?\*/', '', code, flags=re.DOTALL)

    return code


def clean_dict_doxygen_comment(comment):
    # Remove the leading '/**' and trailing '*/'
    comment = comment.strip().lstrip('/**').rstrip('*/').strip()

    # Remove the leading '*' from each line (for multi-line comments)
    lines = comment.split('\n')
    stripped_lines = [line.lstrip(' *').strip() for line in lines]

    # Join the lines back into a single string
    return '\n'.join(stripped_lines)


def extract_dict_doxygen_comments(file_path):
    # Regular expression to match Doxygen comments
    doxygen_comment_pattern = re.compile(r'/\*\*(?!<).*?\*/', re.DOTALL)

    # Regular expression to match the typedef for a struct ending with '_Dict_type'
    typedef_pattern = re.compile(
        r'typedef\s+struct(?:\s+\w+)?\s*{[^}]*}\s+(\w+Dict_type)\s*;',
        re.DOTALL
    )

    # Dictionary to hold the results: struct name -> Doxygen comment
    results = {}

    with open(file_path, 'r') as file:
        content = file.read()

        content = remove_header_comments(content)

        # Find all Doxygen comments in the file
        doxygen_comments = doxygen_comment_pattern.findall(content)

        # Find all typedefs of structs ending with '_Dict_type'
        typedefs = typedef_pattern.findall(content)

        # Initialize variables to track start/end pos for structs
        start_pos = 0
        end_pos = 0

        # Iterate through each typedef and associate it with the closest
        # preceding Doxygen comment
        for typedef in typedefs:
            # Add semicolon to differentiate from mentions in comments
            # Alternatively, exclude appearances in comments
            typedef_name_terminated = typedef.strip() + ";"

            # Find the position of the current typedef
            end_pos = content.find(typedef_name_terminated)

            # Find the last occurence of 'typedef struct' before the typedef name
            typedef_pos = content.rfind('typedef struct', start_pos, end_pos)

            # Find end of any previous declaration
            prev_decl_pos = max(0, content.rfind(';', start_pos, typedef_pos))

            start_pos = end_pos

            # Check if there's a valid comment preceding the typedef
            for comment in reversed(doxygen_comments):
                comment_pos = content.find(comment)

                # Only consider the comment if it's before the typedef and after
                # any previous declaration
                if comment_pos < typedef_pos and comment_pos > prev_decl_pos:
                    # Clean up the comment by stripping Doxygen markers
                    stripped_comment = clean_dict_doxygen_comment(comment)
                    # Trim typedef name
                    struct_label = re.sub(r'_type;$', '', typedef_name_terminated)
                    results[struct_label] = stripped_comment
                    break

    return results


def remove_c_comments_except_trailing_doxygen(code):
    """
    Remove block (/* */) and line (//) comments,
    but preserve Doxygen trailing member comments: /**< ... */
    """
    # First remove block comments that are NOT doxygen trailing comments
    code = re.sub(r'/\*\*(?!<).*?\*/', '', code, flags=re.DOTALL)
    # Remove all line comments
    code = re.sub(r'//.*', '', code)
    return code


def clean_member_doxygen_comment(comment):
    """
    Cleans up the Doxygen comment by removing the Doxygen markers,
    normalizing newlines, and trimming unnecessary spaces.
    """
    # Remove Doxygen start markers (/**< or ///<)
    comment = re.sub(r'(^/\*\*<|^///<|\*/)', '', comment.strip()).strip()

    # Replace newlines and excessive spaces with a single space
    comment = re.sub(r'\s+', ' ', comment)

    return comment.strip()


def is_blacklisted(var_name):
    """Check if variable name is blacklisted by exact name or suffix."""
    if var_name in blacklist:
        return True
    for suffix in blacklist_suffixes:
        if var_name.endswith(suffix):
            return True
    return False


def extract_dict_type_members_grouped(filename):
    with open(filename, 'r') as f:
        content = f.read()

    content = remove_c_comments_except_trailing_doxygen(content)

    # Match typedef struct { ... } *_Dict_type;
    struct_pattern = re.compile(
        r'typedef\s+struct(?:\s+\w+)?\s*{([^}]*)}\s+(\w+_Dict_type)\s*;'
    )

    matches = struct_pattern.findall(content)
    result = {}

    for struct_body, typedef_name in matches:
        # Remove _type suffix from typedef name
        struct_label = re.sub(r'_type$', '', typedef_name)
        members = []

        # Match struct members with trailing Doxygen comments
        # This handles multi-line comments by allowing any characters
        # (including newlines) within the comment block.
        member_pattern = re.compile(
            r'(\w[\w\s\*]+)\s+(\**\w[\w\d_]+)\s*;(\s*(/\*\*<[\s\S]*?\*/|///<[\s\S]*?)\s*)?'
        )

        matches = member_pattern.findall(struct_body)

        for match in matches:
            member_type = match[0].strip()
            member_name = match[1].strip()
            comment = clean_member_doxygen_comment(match[2]) if match[2] else ""

            if member_name.startswith('*'):
                member_type += '*'
                member_name = member_name.lstrip('*')

            if is_blacklisted(member_name):
                continue

            # handle special vector variable
            if struct_label == 'Data_Dict' and member_name == 'drift_velocity':
                member_type = 'vector'

            member_info = {
                'name': member_name,
                'type': member_type,
                'comment': comment
            }

            members.append(member_info)

        if members:
            result[struct_label] = members

    return result


def extract_brace_block(code, start_index):
    """
    Extract a full {...} block from `code`, starting at `start_index`
    (which must be `{`).
    """
    assert code[start_index] == '{'
    depth = 1
    i = start_index + 1
    while i < len(code) and depth > 0:
        if code[i] == '{':
            depth += 1
        elif code[i] == '}':
            depth -= 1
        i += 1
    return code[start_index + 1:i - 1], i


def extract_default_values(filename):
    with open(filename, 'r') as f:
        content = f.read()

    # Remove comments to avoid confusion
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    content = re.sub(r'//.*', '', content)

    # Get all indices of function bodies
    func_bodies = {}
    for match in re.finditer(r'\s*void\s+default_(\w+_Dict)\s*\((.*?)\)\s*{', content):
        struct_name = match.group(1)
        start_index = match.end() - 1
        body, end_index = extract_brace_block(content, start_index)
        func_bodies[struct_name] = body

    defaults = {}

    for struct_name, body in func_bodies.items():
        struct_defaults = {}

        # Match lines like: s->field = value;
        # Match lines like: s->field[i] = value;
        ptr_assignments = re.findall(
            r'\w+\s*->\s*(\w+)\s*(?:\[\s*.*?\s*\])?\s*=\s*(.*?);',
            body
        )

        # Match lines like: s[i].field = value;
        # Match lines like: s[i].field[j] = value;
        arr_assignments = re.findall(
            r'\w+\s*\[.*?\]\.\s*(\w+)\s*(?:\[\s*.*?\s*\])?\s*=\s*(.*?);',
            body
        )

        # Match lines like: s(i).field = value;
        # Match lines like: s(i).field[j] = value;
        macro_assignments = re.findall(
            r'\w+\s*\(.*?\)\.\s*(\w+)\s*(?:\[\s*.*?\s*\])?\s*=\s*(.*?);',
            body
        )

        assignments = ptr_assignments + arr_assignments + macro_assignments

        for field, value in assignments:
            value = value.strip()

            # If the value contains a pattern like 'ptr->field',
            # remove the pointer part
            value = re.sub(r'(\w+\s*->\s*)*(\w+)', r'\2', value)

            # Exclude NULL pointer assignments
            if value.upper() == 'NULL':
                continue
            # Exclude malloc / calloc / realloc calls
            if re.search(r'\b(malloc|calloc|realloc)\s*\(', value):
                continue

            struct_defaults[field] = value

        # Match strcpy calls like: strcpy(rcfd->s->field, "value");
        strcpy_calls = re.findall(
            r'strcpy\s*\(\s*\w+\s*->\s*\w+\s*->\s*(\w+)\s*,\s*(\".*?\")\s*\)\s*;',
            body
        )

        for field, string_value in strcpy_calls:
            struct_defaults[field] = string_value  # already quoted

        defaults[struct_name] = struct_defaults

    return defaults


def merge_defaults(member_data, default_values):
    for struct_name, members in member_data.items():
        defaults = default_values.get(struct_name, {})
        for member in members:
            name = member["name"]
            if name in defaults:
                member["default"] = defaults[name]


def merge_extra_props(member_data, extra_member_data):
    for extra_struct_name, extra_members in extra_member_data.items():
        member_data[extra_struct_name] += extra_members;


def output_markdown(struct_data, struct_comments):
    ext = ".md"
    for struct_name, members in struct_data.items():
        with open(struct_name.lower()+ext, 'w') as f:
            f.write(f"### {struct_name} {{#{struct_name.lower()}}}\n")
            f.write("\n<!-- This file is auto-generated from source code -->\n")
            if struct_name in struct_comments:
                f.write("\n")
                f.write(struct_comments[struct_name])
                f.write("\n\n")
            f.write("| Dictionary Entry | Type | Default Value | Note |\n")
            f.write("|------------------|------|:-------------:|------|\n")
            for member in members:
                name = member.get("name", "")
                typ = member.get("type", "")
                typ = type_aliases.get(typ, typ)  # Apply alias if exists
                default = member.get("default", "none")
                comment = member.get("comment", "")
                f.write(f"| `\"{name}\"` | {typ} | `{default}` | {comment} |\n")
                # handle variables that can be set via integer or string
                if name in variants or name.endswith("_format"):
                    default_str = re.sub(r'^RCFD_', '', default)
                    underscore_count = max(name.count('_'), 1)
                    for x in range(underscore_count):
                        default_str = re.sub(r'^([a-zA-Z0-9]+_)', '', default_str)
                    default_str = default_str.lower()
                    f.write(f"| `\"{name}\"` "
                            "| string "
                            f"| `\"{default_str}\"` "
                            f"| alternative way to set {name} "
                            "(lowercase enum name without prefix)"
                            "|\n")


# usage
if __name__ == "__main__":
    dicts_comments = extract_dict_doxygen_comments("../src/rCFD_types.h")
    dicts_props = extract_dict_type_members_grouped("../src/rCFD_types.h")
    dicts_props_defaults = extract_default_values("../src/rCFD_defaults.h")
    merge_defaults(dicts_props, dicts_props_defaults)
    merge_extra_props(dicts_props, extra_dict_props)
    output_markdown(dicts_props, dicts_comments)

