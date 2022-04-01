<?php
//利用WS
//參數：ir_no
//呼叫範例：http://duckegg.duckdns.org:8088/getircmd.php?ir_no=bath1
$typ      = $_GET['ir_no'];
$cmd      = $_GET['cmd'];
$serve = 'mysql:host=10.0.4.123:3306;dbname=duckegg;charset=utf8';
$username = 'duckegg';
$password = 'Binomial04!8';

// PDO連線資料庫若錯誤則會丟擲一個PDOException異常
try{
   $PDO = new PDO($serve,$username,$password);
   $query  = "update ircmd             ".
             "   set tsc      = '6' ,  ".
             "       rtt      = now()  ".
             " where ir_no    = '$typ' ".
             "   and cmd      = '$cmd' ".
             "   and tsc      = '1'    ".
             "   and eff_tim <= now()  ";
   $stmt = $PDO->prepare($query);

   // execute the query
   $stmt->execute();
   } catch (PDOException $error){
   echo 'connect failed:'.$error->getMessage();
   }
$PDO = null;
?>
