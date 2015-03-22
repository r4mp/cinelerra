# A proposed list of Cinelerra future tasks

# Introduction #

rpm spec for suse, fedora, centos (prototype, needs alot of work)
deb for ubuntu (not started)
nix build messages (low priority, but helps keep the build stable, not started)
OpenGL direct linkage instead of single threaded strategy.
V4L2 close device in missing on close, needs attention (checking, need ntsc)
cosmetic change remove set\_sync(0) in assetedit.C
add session logging preference
check youtube file access for api
check pidgin for integration and api
update build prequisites for new build requirements (ongoing)
add db path setup to preferences
update documentation (ongoing)
check out amd renderfarm for 1K+ cpu ultra-HD show
VFrame force no shm constructor option
make custom glyph (color images) available to titler
Android remote control app
make remote key symbols mappable
autoscale icon data size to match screen resolution
dangling threads (ongoing, low priority)
need a good way to characterize image for DB key. (the truth is out there)
CV Makefiles
rgba color model works badly in titler color selector
svg plugin segv on access file, missing client->defaults
check yuv422 colormodel mpeg output transcode, reworked untested
check OGG format, PackageRender is new
check fileYUV formats, ffmpeg.C pipe.C yuvstream.C are new, filedv is new
File has new\_packaging\_engine(Asset 