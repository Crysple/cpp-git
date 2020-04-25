#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
#include <set>
#include <unordered_map>
// To get the data structures
#include <git_objects.hpp>
/* #include <helper.hpp> */

/* ********* Global Variables ********** */
const std::set<std::string> CAT_FILE_SUBCMDS = {"blob", "commit", "tag", "tree"};
#define CAT_FILE_USAGE "usage: git cat-file <type> <object>\n<type> can be one of: blob, tree, commit, tag"
#define HASH_OBJECT_USAGE "usage: git hash-object <type> <path>\n<type> can be one of: blob, tree, commit, tag"
#define GIT_INIT_USAGE "usage: git init <dir>\nCreate an empty Git repository as <dir>. <dir> defaults to be '.'"
#define CHECKOUT_USAGE "usage: git checkout [<branch>/<commit>]"

namespace fs = std::filesystem;

/* ********* Functions	********* */
// Functions dealing with commands: validate, convert file path to absolute etc.
void cmd_init(const std::vector<std::string>& args);
void cmd_cat_file(const std::vector<std::string>& args);
void cmd_checkout(const std::vector<std::string>& args);
void cmd_show_ref(const std::vector<std::string> &args);
void cmd_hash_object(const std::vector<std::string> &args);
// Actual functions executing commands
void git_cat_file(fs::path obj, const std::string& fmt);
void git_init(fs::path project_base_path);
void git_checkout(std::string hash);

int test_function(void);

std::string git_add_file(const fs::path& file_path);
std::string read_project_folder_and_write_tree(const fs::path& adding_directory, bool root = false);
std::string git_add_folder(const fs::path folder_path);

std::string ref_resolve(fs::path path);
std::unordered_map<std::string, std::string> ref_list(fs::path base_path);
#endif
