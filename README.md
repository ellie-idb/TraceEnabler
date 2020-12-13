# TraceEnabler
This is a DLL for Blockland. It enables faster tracing of functions (created for Conan's Farming)
## Usage
This function hooks `CodeBlock::exec`. As such, you'll have to load it very early on (before `Sim::init`, preferably).

