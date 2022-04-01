<?php
$dev = $_GET['dev'];
$val = $_GET['val'];

$servername = "10.0.4.123";
$username = "duckegg";
$password = "Binomial04!8";
$dbname = "duckegg";

$sql = "INSERT INTO ircmd(cmd_no, ir_no,cmd,tsc,rtt) VALUES (0,'$dev','$val','1',now())";

echo $sql.' <br><br>';

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}


if ($conn->query($sql) === TRUE) {
    echo "Record updated successfully";
} else {
    echo "Error updating record: " . $conn->error;
}

$conn->close();
?>
