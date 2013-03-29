del *.exe
del *.wixobj
del *.wixpdb
del *.msi
copy ..\Bundle_Launch.exe .\Launch.exe
"C:\Program Files\WiX Toolset v3.7\bin\candle.exe" bundles.wxs
"C:\Program Files\WiX Toolset v3.7\bin\light.exe" -ext WixUIExtension bundles.wixobj
