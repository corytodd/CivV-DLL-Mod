set BASE_DLL="%CD%\BuildOutput\VS2008_ModWin32\CvGameCoreDLLFinal Release.dll"
set EXP_DLL1="%CD%\BuildOutput\VS2008_ModWin32\CvGameCore_Expansion1.dll"
set EXP_DLL2="%CD%\BuildOutput\VS2008_ModWin32\CvGameCore_Expansion2.dll"

copy %BASE_DLL% "C:\progra~2\Steam\steamapps\common\Sid Meier's Civilization V\"
copy %EXP_DLL1% "C:\progra~2\Steam\steamapps\common\Sid Meier's Civilization V\Assets\DLC\Expansion"
copy %EXP_DLL2% "C:\progra~2\Steam\steamapps\common\Sid Meier's Civilization V\Assets\DLC\Expansion2"