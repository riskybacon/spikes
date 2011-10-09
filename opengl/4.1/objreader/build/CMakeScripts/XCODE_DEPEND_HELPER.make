# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# For each target create a dummy rule so the target does not have to exist
/usr/local/lib/libglfw.a:


# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.objreader.Debug:
/Users/jbowles/spikes/opengl/4.1/objreader/build/Debug/objreader:\
	/usr/local/lib/libglfw.a
	/bin/rm -f /Users/jbowles/spikes/opengl/4.1/objreader/build/Debug/objreader


PostBuild.objreader.Release:
/Users/jbowles/spikes/opengl/4.1/objreader/build/Release/objreader:\
	/usr/local/lib/libglfw.a
	/bin/rm -f /Users/jbowles/spikes/opengl/4.1/objreader/build/Release/objreader


PostBuild.objreader.MinSizeRel:
/Users/jbowles/spikes/opengl/4.1/objreader/build/MinSizeRel/objreader:\
	/usr/local/lib/libglfw.a
	/bin/rm -f /Users/jbowles/spikes/opengl/4.1/objreader/build/MinSizeRel/objreader


PostBuild.objreader.RelWithDebInfo:
/Users/jbowles/spikes/opengl/4.1/objreader/build/RelWithDebInfo/objreader:\
	/usr/local/lib/libglfw.a
	/bin/rm -f /Users/jbowles/spikes/opengl/4.1/objreader/build/RelWithDebInfo/objreader


