# OpenDSS-X

This repository contains OpenDSS version 10 (X), which is a translation from
Embarcadero Delphi into C++.

This repository includes other git repositories as submodules.  To clone this
repository and include all submodules, use `--recurse-submodules` when cloning.

For example, here is one way to clone and build this project:
```
git clone --recurse-submodules git@gitlab.epri.com:pdmo005/opendss-x
```

In case this repo was already cloned without `--recurse-submodules`, then the
submodules may still be initialized and fetched with:
```
git submodule update --init --remote --recursive
```

For more information on working with submodules, please see
https://git-scm.com/book/en/v2/Git-Tools-Submodules
