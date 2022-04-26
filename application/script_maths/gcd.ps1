<#
.DESCRIPTION
求1 2 3 4 5 6 7 8 9 九个数组组成不重复九位数的最大公约数
#>

$prime = new-primetab 32000
$num = (1,2,3,4,5,6,7,8,9);

function convert_array_to_string($arr)
{
	$str = "{";
	$arr | %{$str += "$_, "};
	$str += "}";
	$str;
}

<#
.DESCRIPTION
质因数分解。需要输入质数表。
#>
function factor($val, $prime=$global:prime)
{
#	$ff = new-object System.Collection.ArrayList;
	$ff =@();
	$pp =0;
	write-debug "prime count = $($prime.count), input = $val"
	while ( ($val -gt 1) -and ($pp -lt $prime.count))
#	while (  ($pp -lt $prime.count))
	{
#		write-debug "check prime $($prime[$pp]), input=$val"
		if ( ($val % $prime[$pp]) -eq 0)
		{
			write-debug "factor = $($prime[$pp])"
			$ff += $prime[$pp];
			$val = $val / $prime[$pp];
		}
		else {$pp ++;}
	}
	if ($val -gt 1) {$ff += $val;}
	$ff;
}

<#
.DESCRIPTION
合成：将给定的数组合成一个多位整数
#>

function combine($set)
{
	[int]$res = 0;
	foreach ($a in $set)
	{
		$res *= 10;
		$res += [int]$a;
	}
	return $res;
}

<#
.DESCRIPTION
求一个数的各个数字之和
#>
function digi_count($val)
{
	$res=0;
	while ($val -gt 0)
	{
		$res += ($val %10);
		$val = [math]::floor($val/10);
	}
	return $res;
}

function sum($a)
{
	$s=0;
	$a | %{$s+=$_;}
	$s;
}

function q11
{
	for ($ii=1; $ii -le 100; $ii++)
	{
		$s=0;
		(1..$ii) | %{$s+=(1/$_);}
		write-host "$ii : $s";
		if ($s > 3) {break;}
		
	}
}


<#
#factor -val 123456789 -prime $prime
"" > output.txt
new-permutation -input $num -count 9 | 
#	select-object -first 10 | 
	%{	$nn = $_.set;
	$vv = (((((((($nn[0]*10 + $nn[1])*10 + $nn[2])*10 + $nn[3])*10 + $nn[4])*10 + $nn[5])*10 + $nn[6])*10 + $nn[7])*10+ $nn[8]); 
	$vv;	
} |	%{
	$f = factor -val $_ -prime $prime;
	"{0}: {1}" -f $_, $(convert_array_to_string $f) >> output.txt
#	$f | out-file output.txt -append
	}
#>

function lamps($count)
{
	$lights = @();
	for ($ii=0; $ii -le $count; $ii++) {$lights += 0;};
	for ($ii=1; $ii -le $count; $ii++) 
	{
		#翻转
		for ($jj=$ii; $jj -le $count; $jj+=$ii) {$lights[$jj] = 1- $lights[$jj];}
		#输出
		$out = "{0:d03}:" -f $ii;
		for ($jj=1; $jj -le $count; $jj++) {$out += "{0}" -f $lights[$jj];}
		write-host $out;
	}
	#统计
	$bright=@();
	for ($ii=1; $ii -le $count; $ii++)
	{
		if ($lights[$ii] -eq 1) {$bright += $ii;}
	}
	$bright;
}
