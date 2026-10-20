# Discord blurb (copy/paste)

Community Digmesa measured-flow controller build (not official GaggiMate):

**Repo:** https://github.com/dulerabbit/gaggimate-maxa4turbo-flow  
**Release:** https://github.com/dulerabbit/gaggimate-maxa4turbo-flow/releases

- Flash **Pro controller only** (bootloader + partitions + firmware from the Release). Leave your display alone.
- Wire Digmesa signal → **EXT4 (GPIO12)** + pull-up to 3.3V. Not IO14, not Velofuso TCA IO2.
- FLOW mode = PI on measured ml/s; pressure target is still the ceiling.

Details in the README.
