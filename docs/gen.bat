@echo off
echo y | rd /s C:\Chris\MyProjects\FFRend\web\Help
ren helpframe.html helpframe.txt
attrib +r gallery\*.* /s
navgen template.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
attrib -r gallery\*.* /s
cd gallery
navgen template.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
cd..
if errorlevel 1 goto err
ren helpframe.txt helpframe.html
md Help
C:\Chris\MyProjects\doc2web\release\doc2web /nospaces C:\Chris\MyProjects\FFRend\Help\help.txt Help Contents.htm C:\Chris\MyProjects\FFRend\doc\FFRendHelp.htm "FFRend Help"
if errorlevel 1 goto err
cd Help
md images
copy C:\Chris\MyProjects\FFRend\Help\images\*.* images
copy ..\helptopic.css content.css
navgen C:\Chris\MyProjects\FFRend\Help\template.txt .
copy ..\helpheader.txt x
copy x + Contents.htm
echo ^<body^>^<html^> >>x
del Contents.htm
ren x Contents.htm
md printable
cd printable
move C:\Chris\MyProjects\FFRend\doc\ffrendhelp.htm .
echo y|fsr ffrend~1.htm "<head>" "<head><style type=\"text/css\">.cmd {font-style: italic} .key {font-family: Courier}</style>"
cd ..
copy C:\Chris\MyProjects\FFRend\doc\ffrendhelp.doc printable
copy C:\Chris\MyProjects\FFRend\doc\ffrendhelp.pdf printable
goto exit
:err
pause Error!
:exit
