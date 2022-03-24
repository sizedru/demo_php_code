<?php


Autoloader::Register();
set_time_limit(0);

function LOG_log($a, $s, $o){
    $file_res="log.log";
    $ss = mb_substr($s."                                                                                             ", 0, 60);
    file_put_contents($file_res, date('Y-m-d H:i:s') . " | " . substr($a."          ", 0, 8)." | ". $ss . " | ". $o. "\n", FILE_APPEND);
}

function getWEBResource($url)
{
    $options = [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_HEADER => false,
        CURLOPT_FOLLOWLOCATION => true,
        CURLOPT_ENCODING => "",
        CURLOPT_USERAGENT => "RDFinance",
        CURLOPT_AUTOREFERER => true,
        CURLOPT_CONNECTTIMEOUT => 10,
        CURLOPT_TIMEOUT => 10,
        CURLOPT_MAXREDIRS => 10,
        CURLOPT_SSL_VERIFYHOST => 0,
        CURLOPT_SSL_VERIFYPEER => 0
    ];

    $ch = curl_init($url);
    curl_setopt_array($ch, $options);
    $content = curl_exec($ch);
    $err = curl_errno($ch);
    $errmsg = curl_error($ch);
    $header = curl_getinfo($ch);
    curl_close($ch);

    $header['errno'] = $err;
    $header['errmsg'] = $errmsg;
    $header['content'] = $content;

    return $header;
}


$dry=false;
if($argc>1){
    $dry=true;
}

$SQL="
    select .......... order_id;
;";

echo "RUN\n";

$redis = new Redis();
$redis->connect('127.0.0.1', 6379);

$NeedSendCallCancel = false;

echo "\nANDERS TOOK ORDER:\n";
// Выбираем все принятые заявки
$arList = $redis->keys("took_ander_order*");
foreach($arList as $key=>$val){
    $k=intval($redis->get($val));
    $v=str_replace("took_ander_order_", "", $val);

    Log_log($v, "заявка взята пользователем", $k);

    $NeedSendCallCancel = true;

    $redis->del($val);
}

echo "\nANDERS ACTIVE:\n";
// Выбираем всех активных Андеров
$arList = $redis->keys("ander*");
$arActiveIds = array();
foreach($arList as $key=>$val){
    $k=intval($redis->get($val));
    $v=str_replace("ander_", "", $val);
    $arActiveIds[$v] = $k;
}

print_r($arActiveIds);

echo "\nANDERS VOTES:\n";

// Выбираем всех ожидающих Андеров
$arList = $redis->keys("vote_ander*");
$arVoteIds = array();
foreach($arList as $key=>$val){
    $k=intval($redis->get($val));
    $v=str_replace("vote_ander_", "", $val);
    if(isset($arActiveIds[$v])){
        $arVoteIds[$k] = $v;
    }else{
        echo "vote ".$v." is inactive - delete \n";
    }
    
}

foreach($arActiveIds as $key=>$val){
    if(!array_search($key, $arVoteIds)){
        Log_log($key, "отключил прием заявок", "");
    }
}

ksort($arVoteIds);
print_r($arVoteIds);

echo "\nORDERS FOR ANDERS:\n";

$NeedSendCall = false;

$arList = $redis->keys("order_for_ander*");
$arOrdersFor = array();
foreach($arList as $key=>$val){
    //echo "$val=>".$redis->get($val)."\n";
    $k=intval($redis->get($val));
    $v=str_replace("order_for_ander_", "", $val);
    $arOrdersFor[$v] = $k;
    Log_log($v, "заявка НЕ взята пользователем, ожидаем", $k);
    $NeedSendCall = true;
}

if(date('H')>="00" && date('H')<"08"){
    $flagCallAnderTimeoutExist = $redis->exists('call_ander_timeout') == 1;
    $flagCallAnderNeed = $redis->exists('call_for_ander_need') == 1;
    $flagCallAnderAlreadySended = $redis->exists('call_for_ander_exist') == 1;
    
    if($NeedSendCallCancel){
        if($flagCallAnderTimeoutExist){
            $redis->del('call_ander_timeout');
        }
    
        if($flagCallAnderNeed){
            $redis->del('call_for_ander_need');
            Log_log("00000", "отключаем отправку звонка, есть кто-то живой", "000000");
        }
    
        if($flagCallAnderAlreadySended) {
            $redis->del('call_for_ander_exist');
        }
    }else if($NeedSendCall){
        // Отправляем ночной звонок
            if($flagCallAnderAlreadySended){
                Log_log("00000", "звонок уже отправлен ранее, прошло меньше 3 минут", "000000");
            }else{
                if($flagCallAnderTimeoutExist) {
                    $redis->setEx('call_for_ander_need', 100, $phone);
                    if($flagCallAnderNeed) {
                        Log_log("00000", "подтверждаем отправку звонка после таймаута", "000000");
                    }else{
                        Log_log("00000", "включаем отправку звонка", "000000");
                    }
                } else if($flagCallAnderNeed){
                    Log_log("00000", "отправляем звонок - уведомление сотрудников о заявке", "000000");
                    // ..........
                    // ..........
                    // ..........
                    // ..........
                    // ..........
    
                    if($flagCallAnderTimeoutExist){
                        $redis->del('call_ander_timeout');
                    }
    
                    if($flagCallAnderNeed){
                        $redis->del('call_for_ander_need');
                    }
    
                } else {
                    $redis->setEx('call_ander_timeout', 180, $phone);
                    Log_log("00000", "включаем таймаут отправки звонка", "000000");
                }
            }
    }
}

print_r($arOrdersFor);

echo "\nORDERS:\n";

$arOrdersClean = array();
$arOrdersBusy = array();

$rs = DB::select($SQL);
while ($rs->next()) {
    $oid=$rs->get('order_id');
    if($rs->get('auh_user') > 0){
        if(isset($arActiveIds[$rs->get('auh_user_login')])){
            $arOrdersBusy[$oid] = $rs->getRow(); 
            echo "$oid BUSY ".$rs->get('auh_user_login')."\n";
        }else if($rs->get('last')<20){
            $arOrdersBusy[$oid] = $rs->getRow(); 
            echo "$oid ALREADY BUSY ".$rs->get('auh_user_login')."\n";
        }else {
            $arOrdersClean[$oid] = $rs->getRow(); 
            echo "$oid ALREADY CLEAN\n";
        }
    }else{
        $arOrdersClean[$oid] = $rs->getRow(); 
        echo "$oid CLEAN\n";
    }
}

//print_r($arOrders);

//exit;

$c=count($arVoteIds);
if($c > 0){
    echo "\nORDERS DELETE:\n";
    foreach($arOrdersClean as $key=>$val){
        if(array_search($key, $arOrdersFor)){
            unset($arOrdersClean[$key]);
            echo "$key\n";
        }
    }

    foreach($arOrdersBusy as $key=>$val){
        if(array_search($key, $arOrdersFor)){
            unset($arOrdersBusy[$key]);
            echo "$key\n";
        }
    }

    echo "\nUSERS DELETE:\n";
    foreach($arOrdersFor as $key=>$val){
        $k=array_search($key, $arVoteIds);
        if($k){
            unset($arVoteIds[$k]);
            echo "$key\n";
        }
    }


    $oc=count($arOrdersClean);
    $ob=count($arOrdersBusy);
    if($oc > 0 || $ob > 0){
        echo "\nADD ORDER BUSY:\n";

        $u=count($arVoteIds);
        if($ob > 0 && $u > 0){
            foreach($arOrdersBusy as $key=>$val){
                $k=array_search($val['auh_user_login'], $arVoteIds);
                if($k){
                    unset($arVoteIds[$k]);
                    unset($arOrdersBusy[$key]);


                    if($dry){
                        echo "DRY RUN ";
                    }        
    
                    echo "add in vote ".$val['auh_user_login']." is busy ".$key."\n";
                    if(!$dry){
                        $redis->setEx('povtor_order_for_ander_'.$val['auh_user_login'],120, $key);    
                        $redis->setEx('order_for_ander_'.$val['auh_user_login'],120, $key);    
                    }
                    Log_log($val['auh_user_login'], "заявка вернулась с доработки", $key);
                }            
            }
        }

        echo "\nADD ORDER:\n";

        $u=count($arVoteIds);
        $oc=count($arOrdersClean);
        if($oc > 0 && $u > 0){
            $i=0;

            list($usec, $sec) = explode(" ", microtime());
            $sec = intval($sec);

            $onlyOrderKeys = array_keys($arOrdersClean);
            foreach($arVoteIds as $key=>$val){
                $SQL = "select .... login='".$val."')";
                $rs = DB::select($SQL);
                if($rs->next()) {
                    $cid=$rs->get('cid');
                    if($cid>=6){
                        echo "user take ".$val." over 6 orders\n";
                        Log_log($val, "взял уже 6 заявок и не может больше брать", "");
                        // $redis->setEx('vote_ander_'.$val,120, $sec+20);
                        // $sec++;         
                        continue;
                    }
                }
                if($i>=$oc){
                    break;
                }

                if($dry){
                    echo "DRY RUN ";
                }        

                $last_orders_count = $arOrdersClean[$onlyOrderKeys[$i]]['last_orders_count'];

                echo "add in vote ".$val." new ".$onlyOrderKeys[$i]." ".$last_orders_count."\n";
                if(!$dry){
                    $redis->setEx('order_for_ander_'.$val,240, $onlyOrderKeys[$i]);

                    $SQL="INSERT IGNORE ................");";
                    DB::execute($SQL);
                }

                if($last_orders_count > 0){
                    Log_log($val, "предложена новая заявка (повторник)", $onlyOrderKeys[$i]);
                }else{
                    Log_log($val, "предложена новая заявка (первичник)", $onlyOrderKeys[$i]);
                }

                $redis->setEx('vote_ander_'.$val,120, $sec);
                $sec++; 

                $i++;
            }
        }
    }

}


?>
