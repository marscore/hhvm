<?php
function Test($param) {

	ZendGoodExtSoapTestsBugsBug38055::$g1 = $param->boolA;
	ZendGoodExtSoapTestsBugsBug38055::$g2	= $param->boolB;
	return 1;
}

class TestSoapClient extends SoapClient {
  function __construct($wsdl) {
    parent::__construct($wsdl);
    $this->server = new SoapServer($wsdl);
    $this->server->addFunction('Test');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
    ob_start();
    $this->server->handle($request);
    $response = ob_get_contents();
    ob_end_clean();
    return $response;
  }
}

$client = new TestSoapClient(dirname(__FILE__).'/bug38055.wsdl');
$boolA = 1;
$boolB = '1';
$res = $client->Test(array('boolA'=>$boolA, 'boolB'=>$boolB));
var_dump(ZendGoodExtSoapTestsBugsBug38055::$g1);
var_dump(ZendGoodExtSoapTestsBugsBug38055::$g2);

abstract final class ZendGoodExtSoapTestsBugsBug38055 {
  public static $g1;
  public static $g2;
}
