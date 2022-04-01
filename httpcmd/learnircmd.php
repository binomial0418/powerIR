<form method="post">
    <input type ="text"  name="name" id="name" value="" style="font-size:40px;">
    <input type="submit" name="stop" id="stop" value="stop learn" style="font-size:40px;"><br/> 
    <input type="submit" name="StandBy" id="StandBy" value="StandBy" style="font-size:40px;">
    <input type="submit" name="AirH" id="AirH" value="AirH" style="font-size:40px;"><br/>
    <input type="submit" name="DryHot" id="DryHot" value="DryHot" style="font-size:40px;">
    <input type="submit" name="DryCold" id="DryCold" value="DryCold" style="font-size:40px;"><br/>
    <input type="submit" name="AirCdrPower" id="AirCdrPower" value="AirCdrPower" style="font-size:40px;">
    <input type="submit" name="AirCdrSendCold" id="AirCdrSendCold" value="AirCdrSendCold" style="font-size:40px;"><br/>
    <input type="submit" name="AirCdrSendAir" id="AirCdrSendAir" value="AirCdrSendAir" style="font-size:40px;">
    <input type="submit" name="AirCdrUP" id="AirCdrUP" value="AirCdrUP" style="font-size:40px;"><br/>
    <input type="submit" name="AirCdrDOWN" id="AirCdrDOWN" value="AirCdrDOWN" style="font-size:40px;">
    <input type="submit" name="AirCdrFlow" id="AirCdrFlow" value="AirCdrFlow" style="font-size:40px;"><br/>
    <input type="submit" name="TvPower" id="TvPower" value="TvPower" style="font-size:40px;">
    <input type="submit" name="TvMute" id="TvMute" value="TvMute" style="font-size:40px;"><br/>

</form>

<?php

function testfun()
{
   echo "Your test function on button click is working".$_POST['name'];
}

if(array_key_exists('StandBy',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanStandBy";
   header("location:".$s);
}
if(array_key_exists('AirH',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirH";
   header("location:".$s);
}
if(array_key_exists('DryHot',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanDryHot";
   header("location:".$s);
}
if(array_key_exists('DryCold',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanDryCold";
   header("location:".$s);
}
if(array_key_exists('AirCdrPower',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirCdrPower";
   header("location:".$s);
}
if(array_key_exists('AirCdrSendCold',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirCdrSendCold";
   header("location:".$s);
}
if(array_key_exists('AirCdrSendAir',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirCdrSendAir";
   header("location:".$s);
}
if(array_key_exists('AirCdrUP',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirCdrUP";
   header("location:".$s);
}
if(array_key_exists('AirCdrDOWN',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirCdrDOWN";
   header("location:".$s);
}
if(array_key_exists('AirCdrFlow',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanAirCdrFlow";
   header("location:".$s);
}
if(array_key_exists('TvPower',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanTvPower";
   header("location:".$s);
}
if(array_key_exists('TvMute',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=LeanTvMute";
   header("location:".$s);
}
if(array_key_exists('stop',$_POST)){
   $s="http://www.inskychen.com:8088/homesys/httpcmd/setcmd.php?dev=".$_POST['name']."&val=StopLean";
   header("location:".$s);
}

?>
