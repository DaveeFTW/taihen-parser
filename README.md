# taihen-parser - _taiHEN's configuration parser_

taiHEN is a custom firmware (CFW) framework for PS Vita™ and implements a configuration to help packagers and users control which modules or plugins are loaded and when.
taihen-parser provides a convenient C API for interacting with these configuration files to help developers write supporting tools for taiHEN. taihen-parser provides both a lexer and parser API for configuration files.

The problem with CFW of a previous era was that it was one person's imagination of a custom firmware. Average developer X could not easily replace the in-game menu within the provided CFW. Likewise, average user Y could not strip out features they did not like. Here, taiHEN provides a solution by providing a configuration file where CFW can be defined as a set of modules and plugins. 

Person X may like live-area mod1 and person Y may like live-area mod2. No longer do these two need to chose between CFW A and CFW B that implements mod1 and mod2, respectively. Instead, they can modify this configuration from their favourite CFW to use whichever mod they prefer. This architecture promotes the _custom_ in _custom firmware_ by encouraging developers to move away from huge monolithic CFW of the past and help nurture an open, compatible and _user orientated_ custom firmware experience.

## Configuration Format
taiHEN employs a text based format for configuring automatic loading of modules. The configuration format is a UTF-8 text file that utilises line seperation to ease parsing and human readability. Each line must be exclusive to one of four types:
 - An empty line
 - A comment
 - Section
 - Module path

Each line can be at most ```CONFIG_MAX_LINE_LENGTH``` characters wide, and trailing/leading whitespace is permitted.

## Lexer Tokens
The config lexer produces the following tokens:
 - ```CONFIG_START_TOKEN```
 - ```CONFIG_END_TOKEN```
 - ```CONFIG_COMMENT_TOKEN```
 - ```CONFIG_SECTION_TOKEN```
 - ```CONFIG_SECTION_HALT_TOKEN```
 - ```CONFIG_SECTION_NAME_TOKEN```
 - ```CONFIG_PATH_TOKEN```

A valid configuration format should obey the grammar:
```
config ::= CONFIG_START_TOKEN (CONFIG_COMMENT_TOKEN | section)* CONFIG_END_TOKEN
section ::= CONFIG_SECTION_TOKEN CONFIG_SECTION_HALT_TOKEN? CONFIG_SECTION_NAME_TOKEN ('\n' | EOF) path*
path ::= CONFIG_PATH_TOKEN ('\n' | EOF)
```

## Sections: ```*```
A section in the configuration file functions as a filter and controller for CFW module loading.
Each section begins with a ```*``` and can optionally be followed with a ```!``` to mark the section as a halt point (see further below). After these tokens, the rest of the line a UTF-8 name for the section.

A section of the same name may appear in the file multiple times. This functionality is intended to allow users to take advantage of taiHEN's load ordering policy.

### Halt point: ```!```
A section can optionally have the halt point token ```!``` following the section token ```*``` in the configuration file. This token instructs the parser to stop further parsing of the file if the section name is within context. See the examples below for a visual worked case on this feature.

### Reserved names
There are currently two reserved names for sections:
 - ```ALL``` - A catch all user-mode section that will load the modules it contains for every user-mode process.
 - ```KERNEL``` - A section that loads resident kernel modules on the start of taiHEN.

Using the halt point ```!``` on these sections results in undefined behaviour.

## API
This API currently offers no guarantee of stability. Please remember that it may change drastically upon any future versions of taiHEN.
taiHEN's configuration parser exposes it's lexer algorithm to assist in development of supporting tools. Please consult the header files for documentation.

## Example Configurations

Below is an example of a very simple configuration:
```
# example simple config
*ALL
ux0:/plugins/my_plugin.suprx
ux0:/plugins/my_plugin2.suprx
```

This example consists of a single section ```ALL```. Which means that every game/application/homebrew that is launched will have both ```my_plugin.suprx``` and ```my_plugin2.suprx``` loaded in that process space and in order.

More precise functionality may be required for certain homebrew. Perhaps you wish to package your own CFW, in which case you may create a complex configuration as shown below:
```
# hello this is a comment. this line is ignored
   # this line also
        # this too, whitespace at the start of a line is OK
*COOL_GAME
# i'm within a section, woo!
    ux0:/coolgame/plugin.suprx
    # indentation is ok with me
    ux0:/coolgame/plugin2.suprx
	# spaces within path is ok
	ux0:/really cool/I haVe spaces and caps/plugin3.suprx
# next section
*ALL
    # i'm a special section!
    # i'm always included... usually
    ux0:/plugins/ingamemusic.suprx
*KERNEL
    # i'm a special section also!
    # my plugins are loaded to kernel memory as resident modules
    ux0:/taihen/henkaku.skprx
    ux0:/psphacks.skprx
*COOL_GAME
    # this section again?! this is ok! this is a way packagers
    # can take advantage of load order.
    ux0:/coolgame/idependoningamemusic.suprx
*!COOL_GAME2
    # what is the '!' for?
    # the '!' prevents further parsing
    # this would make more sense to put at the start if you want to
    # blacklist certain modules
    # look, nothing to load!
*ALL
    ux0:/plugins/ibreak_coolgame2.suprx
	
	# emojis?
	ux0:/🤔/🦄/👻/🎃.suprx
```
Much more complex, but really I expect even more complexity when real CFW components come around. As mentioned previously, parsing occurs from top to bottom, identical to load order. When parsing, a section context is selected. In the case of taiHEN, this context is a title id such as ```MLCL00001``` for our molecularShell homebrew. In this case, lets assume for ease that we have selected ```COOL_GAME``` and it is a user-mode process.

Comments are ignored, so lets continue until we reach the first section: ```COOL_GAME```. Since our selected section matches this first section, the paths below are loaded until a new section is reached.
 - ```ux0:/coolgame/plugin.suprx```
 - ```ux0:/coolgame/plugin2.suprx```
 - ```ux0:/really cool/I haVe spaces and caps/plugin3.suprx```

Then we reach a new section ```ALL```. As mentioned above, ```ALL``` is a special reserved section name that matches every user-mode process. So our loaded module list grows:
 - ```ux0:/coolgame/plugin.suprx```
 - ```ux0:/coolgame/plugin2.suprx```
 - ```ux0:/really cool/I haVe spaces and caps/plugin3.suprx```
 - ```ux0:/plugins/ingamemusic.suprx```

Next section we reach is the special section ```KERNEL```. This is not processed within our context we so continue until we reach the next section: ```COOL_GAME```. We have already had this section before, but we multiple sections are allowed to take advantage of taiHEN's module load ordering. This is extremely useful when you have dependencies between plugins/modules that need resolved. In this example we have ```idependoningamemusic.suprx``` which must be loaded after ```ingamemusic.suprx```.

Our load list now looks like:
 - ```ux0:/coolgame/plugin.suprx```
 - ```ux0:/coolgame/plugin2.suprx```
 - ```ux0:/really cool/I haVe spaces and caps/plugin3.suprx```
 - ```ux0:/plugins/ingamemusic.suprx```
 - ```ux0:/coolgame/idependoningamemusic.suprx```

Next section is ```COOL_GAME2``` which does not match our section context. This section has a halt point ```!``` but we ignore it in this case because we do not much it.

Lastly, we have the final section ```ALL``` again, which completes our load list:
 - ```ux0:/coolgame/plugin.suprx```
 - ```ux0:/coolgame/plugin2.suprx```
 - ```ux0:/really cool/I haVe spaces and caps/plugin3.suprx```
 - ```ux0:/plugins/ingamemusic.suprx```
 - ```ux0:/coolgame/idependoningamemusic.suprx```
 - ```ux0:/plugins/ibreak_coolgame2.suprx```
 - ```ux0:/🤔/🦄/👻/🎃.suprx```

NOTE: I don't know conclusively if the Vita filesystem supports emojis. Don't use them...

### ```COOL_GAME2``` Halt Point Example
Following the same logic as above, we will walk through the configuration as ```COOL_GAME2``` context.

First section is ```COOL_GAME```, not a match so we skip it.

Second section is ```ALL```, so we load modules from it:
 - ```ux0:/plugins/ingamemusic.suprx```

Third section is ```KERNEL```, so we skip it.

Fourth section is ```COOL_GAME``` again so we skip it.

Fifth section is ```COOL_GAME2``` so we process it. This time we have a halt point so this will be the last section we process. Remember, the halt point ```!``` stops any further parsing. This section however has no modules, so nothing is loaded. A section with no modules is OK. In this case, the following ```ALL``` section breaks ```COOL_GAME2``` in our hypothetical world. By using the halt point correctly, a CFW packager can maximise compatibility whilst maintaining load ordering.

Our final module loading list for ```COOL_GAME2```:
 - ```ux0:/plugins/ingamemusic.suprx```

# Building
To build taihen-parser, you require CMake to generate the appropriate build scripts.
From within the repository directory:
```sh
$ mkdir build && cd build
$ cmake ..
$ make
```

To build the included tests you require the boost ```unit_test_framework``` installed. Then instead use:
```sh
$ mkdir build && cd build
$ cmake -DTEST=ON ..
$ make
```

# Installation
To install to a specified location define ```CMAKE_INSTALL_PREFIX```:
```sh
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/my/install/location ..
$ make
$ make install
```

# Acknowledgements
Team molecule for HENkaku, Yifan Lu for taiHEN and xyz for immense support of the vitasdk.

## License
taihen-parser is licensed under the terms of the MIT license which can be read in the ```LICENSE``` file in the root of the repository.
(C) 2016 David "Davee" Morgan
