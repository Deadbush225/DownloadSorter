#!/usr/bin/env zsh

git add -A
git commit -m "update versioning and included Updater"
git tag -f v1.5.2
git push origin main --tags -f