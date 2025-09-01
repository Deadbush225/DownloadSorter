#!/usr/bin/env zsh

git add -A
git commit -m "attempt to decrease windows installer size"
git tag -f v1.5.2
git push origin main --tags -f