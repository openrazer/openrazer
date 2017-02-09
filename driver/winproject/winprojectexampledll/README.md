# ChromaDLL

Follow the instructions in the ChromaDLL's README for Filter Driver Installation.  

Open ChromaExample.sln in VS2015
Build appropriate version (64 bit debug version is default)

This code opens all Chroma devices it finds, then loops them though the frame custom calls.  WHat this means is that all of your devices that support custom will rapidly change color as long as this is running.