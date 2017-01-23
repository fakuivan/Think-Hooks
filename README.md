# Think-Hooks [![Build Status](https://travis-ci.org/fakuivan/Think-Hooks.svg?branch=master)](https://travis-ci.org/fakuivan/Think-Hooks) [![Build status](https://ci.appveyor.com/api/projects/status/o9ewtn209bp8k3l3/branch/master?svg=true)](https://ci.appveyor.com/project/fakuivan/think-hooks/branch/master)
An extension for SourceMod that exposes hooks to ``IServerGameDLL::Think`` for plugins to use.

## For server admins
You probably were redirected here when trying to install a plugin, so let's be quick.

__Make sure your server is running a version of sourcemod equal or higher than 1.8__

1. Go to the [releases](https://github.com/fakuivan/Think-Hooks/releases) section.
2. Look for the latest post and download the attched zip file for your platform (eg. ``think_hooks-nX-linux.zip`` for Linux).
3. Drag and drop the contents from the file to your ``sourcemod`` folder.
4. Done, load or reload any plugins that need this extension to work, they should ask SourceMod to load the extension for you.

## For scripters
This extension provides hooks to ``IServerGameDLL::Think`` and exposes them via functions that behave similarly to ``OnGameFrame`` and ``RequestFrame``, the only difference is that the ones that this extension provides (``OnServerThink`` and ``RequestThink``) are called even when the server is not generating frames (hibernating), this is specially useful when writing plugins that use forwards triggered by networking events, because these can happen even if the server is on the main game loop or not, and you need to make sure actions are being performed.

###To use this extension:

1. Download the include [``think_hooks.inc``](https://github.com/fakuivan/Think-Hooks/blob/master/include/think_hooks.inc)
2. Place it on your ``include`` folder
3. ``#include "think_hooks.inc"`` from your script

Now you are ready to use ``OnServerThink`` and ``RequestThink``, remember that documentation about these symbols can be found on [``think_hooks.inc``](https://github.com/fakuivan/Think-Hooks/blob/master/include/think_hooks.inc), they are documented using doxygen-like comments, just like all of the sourcemod API. Remember to redirect the end user to [this](https://github.com/fakuivan/Think-Hooks) page every time you publish a project that utilizes this extension.

## Want to contribute?
Fork this repo and issue a pull request.

## Want to request a feature? You found a bug?
Open a new [issue](https://github.com/fakuivan/Think-Hooks/issues) and descibe your request (include as much information as possible).
