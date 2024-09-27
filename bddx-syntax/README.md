#### Transform file ```.bddx``` Syntax Highlight for VS code

**[Node.js](https://nodejs.org/)** is required, it should come with ```npm``` (Node Package Manager).

* Install ```vsce``` (if you haven't already):
  ```npm install -g vsce```
* Package the Extension:
  ```vsce package```
  this will create a ```.vsix``` file.
* Install the Extension
  * Open VS code.
  * Press ```Ctrl+Shift+P``` (or ```Cmd+Shift+P``` on macOS) to open the Command Palette.
  * Search for ```Extensions: Install from VSIX...``` and select it.
  * Browse to the ```.vsix``` file you created and install it.
* Verify the Highlighting
  * View the example ```.bddx``` file in ```../examples```