# Vim/NeoVim Support

This is a manual for Vim/NeoVim users.

## Table of Contents

- [Vim or NeoVim Syntax Highlight Support for `.quest` and `.qsf` Files](#vim-or-neovim-syntax-highlight-support-for-quest-files)
- [CMake Workflow](#cmake-workflow)
  - [cmake-tools.nvim](#cmake-toolsnvim)

## Vim or NeoVim Syntax Highlight Support for `.quest` and `.qsf` Files

To install syntax highlighting for LibMozok `.quest` and `.qsf` formats, copy the `after` directory into your Vim/NeoVim "init" directory (e.g., `[User]/AppData/Local/nvim` on Windows for NeoVim).

## CMake Workflow

The project structure:
- `/build` - use this as the CMake build directory.
- `/src` - contains the main `CMakeLists.txt`. Make sure this is your source directory.

### cmake-tools.nvim

If you're using this plugin:

1. Instead of using `:CMakeGenerate`, do the following:
    - (**Windows** users) at the project root, run:<br>
      ```bash
      cmake -S src -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G "MinGW Makefiles"
      ```
    - (**Unix** users) at the project root, run:<br>
      ```bash
      cmake -S src -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      ```
    - This will properly initialize CMake and generate the `compile_commands.json` file, which is required for proper C++ autocompletion in NeoVim.

2. Now, *from the project's root*, open `nvim`.

3. Run `:CMakeSelectBuildDir`. Set it to `[project_root]/build`.<br>
   Then run `:CMakeSelectCwd` and set it to `[project_root]/src`.<br>
   **Important!** You only need to do this once - the plugin will remember these two settings and restore them the next time you open Neovim from the project's root.

4. You can now use `:CMakeSelectBuildType`, `:CMakeBuild`, `:CMakeInstall`, `:CMakeRunTest`, etc., and enjoy full C++ support.
