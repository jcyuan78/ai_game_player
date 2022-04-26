<#
.DESCRIPTION
时钟问题
#>

<#
.DESCRIPTION
求给定时间的时针与分针的夹角
#>
function clock-angle($time)
{
	if ($time -match "\d+\:\d+")
	{
		$hh = [int]$matches[1];
		$mm = [int]$matches[2];
	}
	#分针的角度
	$am = $mm * 6;
	$ah = $hh*30 + $mm*0.5
	
	$aa = $am-$ah;
	write-host "angle =$aa"
	if ($aa -lt 0) {$aa = $aa+360;}
	elseif ($aa -gt 360) {$aa = $aa-360;}
	return $aa;
}

<#
.DESCRIPTION
给定时间，以及指针的夹角，求时刻
#>
function angle-clock($hh, $angle)
{
	$aa = $hh * 30 - $angle;
	while ($aa -lt 0) {$aa += 360;}
	$str_a = "$aa*2/11";
	write-debug $str_a;
	$mm = update-rational $str_a;
	return $mm.ToString("T");
}


