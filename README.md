# Phlegetron

An audio plugin that provides multiple distortion effects with simple parameter control. The distortion can operate
on two independent channels, either split by a crossover frequency or by harmonic bins. Tweaking the controls should
allow you to tune in to specific frequencies for creating either pleasing or disturbing harmonic distortion, depending
on what takes your fancy.

Phlegetron was built using the [JUCE framework](https://github.com/juce-framework/JUCE).

## The [Issue Tracker](https://github.com/igorski/distortion/issues) is your point of contact

Bug reports, feature requests, questions and discussions are welcome on the GitHub Issue Tracker, please do not send e-mails through the development website. However, please search before posting to avoid duplicates, and limit to one issue per post.

Please vote on feature requests by using the Thumbs Up/Down reaction on the first post.

## Setup

You will need to have CMake and a suitable C compiler installed. JUCE* is a submodule of this repository so
all library dependencies are handled when cloning this repository recursively. When cloning without recursion,
run the following to clone the JUCE framework into a subfolder `JUCE`, relative to this repository root.

```
git clone https://github.com/juce-framework/JUCE
```

_*The version used to create this plugin was [8.0.10](https://github.com/juce-framework/JUCE/releases/tag/8.0.10)_

### Build

You can create a runtime by running the below:

```
cmake . -B build -G <GENERATOR_RUNTIME>
```

where the G flag is optional (defaults to the common generator for your platform) and provided `<GENERATOR_RUNTIME>` should reflect a custom generator to use (e.g. `XCode` (on Mac), `Unix makefiles`, `Ninja`, etc.)

After which you can run:

```
cmake --build
```

### Signing the plugin on macOS

You will need to have your code signing set up appropriately. Assuming you have set up your Apple Developer account, you can find your signing identity like so:

```
security find-identity -p codesigning -v 
```

From which you can take your name and team id (the code between parentheses) and pass them to the build script like so:

```
sh build.sh --team_id TEAM_ID --identity "YOUR_NAME"
```