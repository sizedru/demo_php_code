<?php

namespace rewq;


//error_reporting(E_ERROR | E_PARSE);

error_reporting(E_ALL);


define("DB_CHARSET", 'utf8'); // кодировка бд
define("DB_COLLATE", "");
define("DB_DEBUG", false); // дебаг (true или false)
define("DB_MODE", 2);// 0 - в ручную откр/закр,

define('DB_HOST', '');
define('DB_NAME', '');
define('DB_USER', '');
define('DB_PASSWORD', '');


$db = new \mysqli(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 3306);
$db->set_charset("utf8");
if ($db->connect_error) {
    echo "ERROR CONNECT\n";
    exit;
}

function GenerateSQLForTable($db, $table, $SQL){
    $key_fixed = array('lid_oid_num');

    $rs = $db->query($SQL);
    $exportSQL="";
    while($row=$rs->fetch_assoc()){
        $fields = "";
        $values = "";
        foreach ($row as $field => $value) {
            if($value == null) {
                if(!in_array($field, $key_fixed)) continue;
            }
            $value=str_replace(array("\n","\r","'"), '', $value);
            $fields .= " `" . $field . "`, ";
            $values .= "'" . $value . "', ";

        }
        $fields = trim($fields, ", ");
        $values = trim($values, ", ");
        $insertSQL = "REPLACE INTO `" . $table . "` (".$fields.") VALUES (".$values.")";
        $insertSQL .= ";";
        $exportSQL.=$insertSQL."\n";
    }
    return $exportSQL;
}

function GenerateSQLForOtherTable($db, $table, $where,  $date){
    $SQL = "SELECT * FROM `" . $table . "` WHERE `".$where."` >= '" . $date . "';";
    return GenerateSQLForTable($db, $table, $SQL);
}

echo GenerateSQLForOrdersTable($db, $date);
echo GenerateSQLForOtherTable($db, "p.......", "date", $date);
echo GenerateSQLForOtherTable($db, "c..........", "time", $date);
echo GenerateSQLForOtherTable($db, "k....", "date", $date);
echo GenerateSQLForOtherTable($db, "s........", "date", $date);
echo GenerateSQLForOtherTable($db, "n........", "date", $date);
exit;
