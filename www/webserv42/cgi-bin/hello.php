#!/usr/bin/env php-cgi
<?php

header("Content-Type: text/html");


$name = $_GET["name"] ?? null;
$name = isset($name) && $name !== "" ? htmlspecialchars($name) : "stranger";
$returnPage = getenv('ABSOLUTE_INDEX');

echo "\n";
echo "<html><body><h1>Hello, $name!</h1><br></br><form action=\"/index.html\" method=\"get\"><button type=\"submit\">Go Back</button></form></body></html>";
?>
