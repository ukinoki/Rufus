Les fichiers .vvp s'ouvrent avec Visual Paradigm
https://www.visual-paradigm.com/download/



Le Debugger ne se lance pas sur mac

https://stackoverflow.com/questions/58053623/qt-debugger-using-wrong-python-version-on-mac
As a quick fix, run this in terminal:
defaults write com.apple.dt.lldb DefaultPythonVersion 2
The problem is the lldb's default version is now Python 3. This is causing some issues with Qt that will hopefully get resolved soon:
https://bugreports.qt.io/browse/QTCREATORBUG-22955