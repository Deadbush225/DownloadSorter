- these scripts are meant to be ran at the root path
- build files first

1. Edit release template
2. `bump_version.ps1`
3. Install first using `cmake --install build_folder`
4. `update_installer.ps1` (if changes are made in src)
5. `generate_release_notes.ps1` (makes a release too)
6. commit and push
