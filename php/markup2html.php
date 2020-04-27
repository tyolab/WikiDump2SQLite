<?php


# Define us as being in MediaWiki
define( 'MEDIAWIKI', true );

$wgCommandLineMode = true;

$IP = '/data/mediawiki/dev';

require_once "$IP/includes/parser/Parser.php";
require_once "$IP/includes/normal/UtfNormalUtil.php";
require_once "$IP/includes/GlobalFunctions.php";
require_once "$IP/includes/normal/UtfNormalDefines.php";
require_once "$IP/includes/debug/Debug.php";
// require_once "$IP/includes/dao/IDBAccessObject.php";

require_once "$IP/includes/AutoLoader.php";

require_once "$IP/includes/profiler/Profiler.php";

# Start the profiler
$wgProfiler = array();
if ( file_exists( "$IP/StartProfiler.php" ) ) {
	require "$IP/StartProfiler.php";
}

require_once "$IP/includes/Defines.php";
require_once "$IP/includes/DefaultSettings.php";
require_once "$IP/includes/Setup.php";

spl_autoload_register( array( 'AutoLoader', 'autoload' ) );
require_once "$IP/includes/WikiPage.php";

AutoLoader::loadClass( 'LoadMonitorMySQL' );

$wgTmpDirectory = '/home/monfee/tmp';

// require_once("$IP/extensions/MobileFrontend/MobileFrontend.php");
require_once("$IP/LocalSettings.php");

$wgArticlePath = "/wiki/$1";

$wgUsePathInfo = true;

global $wgContLang; 
global $wgLang;
global $wgMemc;
global $wgParser;
global $wgMaxRedirects;
global $wgUseTidy, $wgAlwaysUseTidy, $wgShowHostnames;
global $params;
global $mainApi;
global $wgRequest;
global $wgUseTidy;

global $mobileViewModuleClass;
global $mobileViewInstance;
global $wgShowExceptionDetails;

$wgShowExceptionDetails = false;
// $wgLang = Language::factory( 'zh' );
// $wgContLang = Language::factory( 'zh' );
// $wgMemc = wfGetMainCache();
// $params = array();

// $wgDBname = "wikipedia";
// $wgDBuser = "root";

$mainApi = new ApiMain( RequestContext::getMain(), false );

$mobileViewModuleClass = "ApiMobileView";
// 		$grpCls = array('action', 'ApiMobileView');

// global $mainApi;
$mobileViewInstance = new $mobileViewModuleClass ( $mainApi );

$mainApi->setupExternalResponse($mobileViewInstance, array('format'=>'json'));

$wgUseTidy = true;

function getCookie( $key, $prefix = null, $default = null ) {
	return isset( $this->cookies[$key] ) ? $this->cookies[$key] : $default;
}

class WikiMarkupToHtml extends ApiBase {
	
	static protected $parserClass = null;
	protected $wgContLang = null;
	protected $options = null;
	
	protected $params = null;

	
	public function __construct() {
		
		
		$class = 'Parser';
		$parserClass = new $class;

		$title = Title::newFromText( 'Title' );
		$params = array();
// 		$wp = $this->makeWikiPage( $title );
// 		$parserOptions = $this->makeParserOptions( $wp );
		

// 		$user = new User();
// 		echo  "Hello, I am here";
// 		$options = new ParserOptions(); //ParserOptions::newFromContext( $this->getContext() ); //ParserOptions::newFromUserAndLang( new User, $wgContLang );
	}
	
	private function buildParams() {
// 		$params[""] = '';
		$params["prop"] = array('text', 'sections', 'normalizedtitle', 'languagecount', 'hasvariants', 'image', 'thumb', 'lastmodified'/* , 'lastmodifiedby' */);
		$params["format"] = 'json';
		$params["noheadings"] = 'yes';
		$params["sectionprop"] = array('level', 'line');
		$params["redirect"] = 'yes';
		$params["sections"] = 'all';
		$params["variant"] = '';
		$params["notransform"] = 'yes';
		$params["onlyrequestedsections"] = 'no';
		$params["offset"] = 0;
		$params["maxlen"] = 0;
		$params["noimages"] = false;
		return $params;
	}
	
	protected function makeParserOptions( WikiPage $wp ) {
		return $wp->makeParserOptions( $this );
	}
	
	protected function getParserOutput( WikiPage $wp, ParserOptions $parserOptions ) {
		wfProfileIn( __METHOD__ );
		$time = microtime( true );
		$parserOutput = $wp->getParserOutput( $parserOptions );
		$time = microtime( true ) - $time;
		if ( !$parserOutput ) {
			wfDebugLog( 'mobile', "Empty parser output on '{$wp->getTitle()->getPrefixedText()}'" .
			": rev {$wp->getId()}, time $time" );
			throw new MWException( __METHOD__ . ": PoolCounter didn't return parser output" );
		}
		$parserOutput->setTOCEnabled( false );
		wfProfileOut( __METHOD__ );
		return $parserOutput;
	}
	
	/**
	 * Creates a WikiPage from title
	 *
	 * @param Title $title
	 * @return WikiPage
	 */
	protected function makeWikiPage( Title $title ) {
		return WikiPage::factory( $title );
	}
	
	private function parseXmlPage($text) {
		$row = array();
		$row["id"] = 1;
		$row["request"] = 11;
		$row["text_id"] = 12;
		$row["text"] = $text;
		$row["parent_id"] = 9;
		
		// the string object of title
		$row["mu2h_page_name"] = 'test';
		
		// the title here should be the title object
// 		$row["title"] = 'test'; 
		
		return $row;
	}
	
	public function convertFromText( $text ) {
		global $mobileViewInstance;
		global $mainApi;
// 		echo $text;

// 		$row[""] = ;
// 		$row[""] = ;
// 		$row[""] = ;
// 		$row[""] = ;
// 		$row[""] = ;
// 		$row[""] = ;
// 		$row[""] = ;
		$row = $this->parseXmlPage($text);
		
		$name = $row["mu2h_page_name"];
		$title = Title::newFromText( $name );
		
		$id = $row["id"];
		$latest = $row["request"];
		
		$title->setArticleID( $id );
		$title->setRevisionID( $latest );
		
		$revision = Revision::newFromRow( $row );
		$wp = $this->makeWikiPage( $title );
		
		$wp->setLastEdit( $revision );
		
		$wp->setId( $id );
		$wp->setLatest( $latest );
		
		$mobileViewInstance->setWikiPage( $wp );
		
		$params = $this->buildParams();
		$params["request"] = $name;
		
		$mobileViewInstance->doWork($params);
		
		$mainApi->printResult( false );
	}
	
	public function convertFromStdin() {
		$fd = fopen("php://stdin", "r");
		$markup = ""; // This will be the variable holding the data.
		while (!feof($fd)) {
			$markup .= fread($fd, 1024);
		}
		fclose($fd);
		
// 		echo $markup;
		$this->convertFromText($markup);
	}
	
	public function execute() {
		
	}
}

// $classWiki2Html = 'WikiMarkupToHtml';
// $wiki2html = new $classWiki2Html;
$classWiki2Html = new ReflectionClass('WikiMarkupToHtml');
$wiki2html = $classWiki2Html->newInstanceArgs(array());

$argc = sizeof($argv);
if ( $argc === 1 )
	$wiki2html->convertFromStdin();
else if ( $argc === 2) {
	$file = file_get_contents($argv[1], true);
	$wiki2html->convertFromText( $file );
}