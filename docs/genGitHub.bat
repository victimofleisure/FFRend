@echo off
echo y | rd /s C:\Chris\MyProjects\FFRend\web\Help
ren helpframe.html helpframe.txt
rem attrib +r gallery\*.* /s
C:\Chris\tools\navgen templateGitHub.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
rem attrib -r gallery\*.* /s
rem cd gallery
rem C:\Chris\tools\navgen template.html .
rem if errorlevel 1 goto err
rem "C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
rem if errorlevel 1 goto err
rem cd..
rem if errorlevel 1 goto err
ren helpframe.txt helpframe.html
md Help
C:\Chris\MyProjects\doc2web\release\doc2web /nospaces C:\Chris\MyProjects\FFRend\Help\help.txt Help Contents.htm C:\Chris\MyProjects\FFRend\doc\FFRendHelp.htm "FFRend Help"
if errorlevel 1 goto err
cd Help
md images
copy C:\Chris\MyProjects\FFRend\Help\images\*.* images
copy ..\helptopic.css content.css
C:\Chris\tools\navgen C:\Chris\MyProjects\FFRend\Help\template.txt .
copy ..\helpheader.txt x
copy x + Contents.htm
echo ^<body^>^<html^> >>x
del Contents.htm
ren x Contents.htm
md printable
cd printable
move C:\Chris\MyProjects\FFRend\doc\ffrendhelp.htm .
echo y | C:\Chris\tools\fsr ffrend~1.htm "<head>" "<head><style type=\"text/css\">.cmd {font-style: italic} .key {font-family: Courier}</style>"
cd ..
copy C:\Chris\MyProjects\FFRend\doc\ffrendhelp.doc printable
copy C:\Chris\MyProjects\FFRend\doc\ffrendhelp.pdf printable
goto exit
:err
pause Error!
:exit
