param($debug = "Debug")
# copy lib to debug
push-location
cd E:\library\opencv\opencv-32\install\lib
mkdir $debug
mv *.lib .\$debug\

# copy dll to debug
cd E:\library\opencv\opencv-32\install\bin
mkdir $debug
mv *.dll .\$debug\
mv *.exe .\$debug\
pop-location
# make hardlink of dee
if ($debug -eq "Debug")
{
	dir E:\library\opencv\opencv-32\install\bin\Debug\*.dll | % {
		$src = resolve-path $_.Name;
		$dst = E:\jingcheng\workspace\dev-for-topai\build\app\DEBUG_DYNAMIC\$_.Name;
		[Link.Creator]::CreateHardLink( $dst, $src, [IntPtr]::Zero)
	}
}
