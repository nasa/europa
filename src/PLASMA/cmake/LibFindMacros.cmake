




<!DOCTYPE html>
<html class="   ">
  <head prefix="og: http://ogp.me/ns# fb: http://ogp.me/ns/fb# object: http://ogp.me/ns/object# article: http://ogp.me/ns/article# profile: http://ogp.me/ns/profile#">
    <meta charset='utf-8'>
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    
    
    <title>cmake-modules/LibFindMacros.cmake at master · Tronic/cmake-modules · GitHub</title>
    <link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="GitHub" />
    <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="GitHub" />
    <link rel="apple-touch-icon" sizes="57x57" href="/apple-touch-icon-114.png" />
    <link rel="apple-touch-icon" sizes="114x114" href="/apple-touch-icon-114.png" />
    <link rel="apple-touch-icon" sizes="72x72" href="/apple-touch-icon-144.png" />
    <link rel="apple-touch-icon" sizes="144x144" href="/apple-touch-icon-144.png" />
    <meta property="fb:app_id" content="1401488693436528"/>

      <meta content="@github" name="twitter:site" /><meta content="summary" name="twitter:card" /><meta content="Tronic/cmake-modules" name="twitter:title" /><meta content="cmake-modules - LibFindMacros development repository and other cool CMake stuff" name="twitter:description" /><meta content="https://avatars3.githubusercontent.com/u/98187?s=400" name="twitter:image:src" />
<meta content="GitHub" property="og:site_name" /><meta content="object" property="og:type" /><meta content="https://avatars3.githubusercontent.com/u/98187?s=400" property="og:image" /><meta content="Tronic/cmake-modules" property="og:title" /><meta content="https://github.com/Tronic/cmake-modules" property="og:url" /><meta content="cmake-modules - LibFindMacros development repository and other cool CMake stuff" property="og:description" />

    <link rel="assets" href="https://assets-cdn.github.com/">
    <link rel="conduit-xhr" href="https://ghconduit.com:25035/">
    

    <meta name="msapplication-TileImage" content="/windows-tile.png" />
    <meta name="msapplication-TileColor" content="#ffffff" />
    <meta name="selected-link" value="repo_source" data-pjax-transient />
      <meta name="google-analytics" content="UA-3769691-2">

    <meta content="collector.githubapp.com" name="octolytics-host" /><meta content="collector-cdn.github.com" name="octolytics-script-host" /><meta content="github" name="octolytics-app-id" /><meta content="4BAA013D:3D65:783F9EE:538BD041" name="octolytics-dimension-request_id" />
    

    
    
    <link rel="icon" type="image/x-icon" href="https://assets-cdn.github.com/favicon.ico" />


    <meta content="authenticity_token" name="csrf-param" />
<meta content="26tJsFIPkad+hGm8c9xz8kzkI9BarLW9SfjCnuKhW2ZmIGU12ivvEG1VRYt09ZCuLmtVbIRZtgbUGZ0T/ZSGHQ==" name="csrf-token" />

    <link href="https://assets-cdn.github.com/assets/github-382e2d2394fe36287509f9d88e1aae81a78b71b2.css" media="all" rel="stylesheet" type="text/css" />
    <link href="https://assets-cdn.github.com/assets/github2-68f308ed3df6360990cad61e748100286688ace6.css" media="all" rel="stylesheet" type="text/css" />
    


    <meta http-equiv="x-pjax-version" content="0a1f007a804ec5e0a6b81786356373f8">

      
  <meta name="description" content="cmake-modules - LibFindMacros development repository and other cool CMake stuff" />

  <meta content="98187" name="octolytics-dimension-user_id" /><meta content="Tronic" name="octolytics-dimension-user_login" /><meta content="20317958" name="octolytics-dimension-repository_id" /><meta content="Tronic/cmake-modules" name="octolytics-dimension-repository_nwo" /><meta content="true" name="octolytics-dimension-repository_public" /><meta content="false" name="octolytics-dimension-repository_is_fork" /><meta content="20317958" name="octolytics-dimension-repository_network_root_id" /><meta content="Tronic/cmake-modules" name="octolytics-dimension-repository_network_root_nwo" />
  <link href="https://github.com/Tronic/cmake-modules/commits/master.atom" rel="alternate" title="Recent Commits to cmake-modules:master" type="application/atom+xml" />

  </head>


  <body class="logged_out  env-production macintosh vis-public page-blob">
    <a href="#start-of-content" tabindex="1" class="accessibility-aid js-skip-to-content">Skip to content</a>
    <div class="wrapper">
      
      
      
      


      
      <div class="header header-logged-out">
  <div class="container clearfix">

    <a class="header-logo-wordmark" href="https://github.com/">
      <span class="mega-octicon octicon-logo-github"></span>
    </a>

    <div class="header-actions">
        <a class="button primary" href="/join">Sign up</a>
      <a class="button signin" href="/login?return_to=%2FTronic%2Fcmake-modules%2Fblob%2Fmaster%2FLibFindMacros.cmake">Sign in</a>
    </div>

    <div class="command-bar js-command-bar  in-repository">

      <ul class="top-nav">
          <li class="explore"><a href="/explore">Explore</a></li>
        <li class="features"><a href="/features">Features</a></li>
          <li class="enterprise"><a href="https://enterprise.github.com/">Enterprise</a></li>
          <li class="blog"><a href="/blog">Blog</a></li>
      </ul>
        <form accept-charset="UTF-8" action="/search" class="command-bar-form" id="top_search_form" method="get">

<div class="commandbar">
  <span class="message"></span>
  <input type="text" data-hotkey="s, /" name="q" id="js-command-bar-field" placeholder="Search or type a command" tabindex="1" autocapitalize="off"
    
    
      data-repo="Tronic/cmake-modules"
      data-branch="master"
      data-sha="c575654e7fe37187606d19774deb0bfe45a06bdd"
  >
  <div class="display hidden"></div>
</div>

    <input type="hidden" name="nwo" value="Tronic/cmake-modules" />

    <div class="select-menu js-menu-container js-select-menu search-context-select-menu">
      <span class="minibutton select-menu-button js-menu-target" role="button" aria-haspopup="true">
        <span class="js-select-button">This repository</span>
      </span>

      <div class="select-menu-modal-holder js-menu-content js-navigation-container" aria-hidden="true">
        <div class="select-menu-modal">

          <div class="select-menu-item js-navigation-item js-this-repository-navigation-item selected">
            <span class="select-menu-item-icon octicon octicon-check"></span>
            <input type="radio" class="js-search-this-repository" name="search_target" value="repository" checked="checked" />
            <div class="select-menu-item-text js-select-button-text">This repository</div>
          </div> <!-- /.select-menu-item -->

          <div class="select-menu-item js-navigation-item js-all-repositories-navigation-item">
            <span class="select-menu-item-icon octicon octicon-check"></span>
            <input type="radio" name="search_target" value="global" />
            <div class="select-menu-item-text js-select-button-text">All repositories</div>
          </div> <!-- /.select-menu-item -->

        </div>
      </div>
    </div>

  <span class="help tooltipped tooltipped-s" aria-label="Show command bar help">
    <span class="octicon octicon-question"></span>
  </span>


  <input type="hidden" name="ref" value="cmdform">

</form>
    </div>

  </div>
</div>



      <div id="start-of-content" class="accessibility-aid"></div>
          <div class="site" itemscope itemtype="http://schema.org/WebPage">
    <div id="js-flash-container">
      
    </div>
    <div class="pagehead repohead instapaper_ignore readability-menu">
      <div class="container">
        

<ul class="pagehead-actions">


  <li>
    <a href="/login?return_to=%2FTronic%2Fcmake-modules"
    class="minibutton with-count star-button tooltipped tooltipped-n"
    aria-label="You must be signed in to star a repository" rel="nofollow">
    <span class="octicon octicon-star"></span>Star
  </a>

    <a class="social-count js-social-count" href="/Tronic/cmake-modules/stargazers">
      1
    </a>

  </li>

    <li>
      <a href="/login?return_to=%2FTronic%2Fcmake-modules"
        class="minibutton with-count js-toggler-target fork-button tooltipped tooltipped-n"
        aria-label="You must be signed in to fork a repository" rel="nofollow">
        <span class="octicon octicon-repo-forked"></span>Fork
      </a>
      <a href="/Tronic/cmake-modules/network" class="social-count">
        0
      </a>
    </li>
</ul>

        <h1 itemscope itemtype="http://data-vocabulary.org/Breadcrumb" class="entry-title public">
          <span class="repo-label"><span>public</span></span>
          <span class="mega-octicon octicon-repo"></span>
          <span class="author"><a href="/Tronic" class="url fn" itemprop="url" rel="author"><span itemprop="title">Tronic</span></a></span><!--
       --><span class="path-divider">/</span><!--
       --><strong><a href="/Tronic/cmake-modules" class="js-current-repository js-repo-home-link">cmake-modules</a></strong>

          <span class="page-context-loader">
            <img alt="" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
          </span>

        </h1>
      </div><!-- /.container -->
    </div><!-- /.repohead -->

    <div class="container">
      <div class="repository-with-sidebar repo-container new-discussion-timeline js-new-discussion-timeline  ">
        <div class="repository-sidebar clearfix">
            

<div class="sunken-menu vertical-right repo-nav js-repo-nav js-repository-container-pjax js-octicon-loaders">
  <div class="sunken-menu-contents">
    <ul class="sunken-menu-group">
      <li class="tooltipped tooltipped-w" aria-label="Code">
        <a href="/Tronic/cmake-modules" aria-label="Code" class="selected js-selected-navigation-item sunken-menu-item" data-hotkey="g c" data-pjax="true" data-selected-links="repo_source repo_downloads repo_commits repo_releases repo_tags repo_branches /Tronic/cmake-modules">
          <span class="octicon octicon-code"></span> <span class="full-word">Code</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

        <li class="tooltipped tooltipped-w" aria-label="Issues">
          <a href="/Tronic/cmake-modules/issues" aria-label="Issues" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-hotkey="g i" data-selected-links="repo_issues /Tronic/cmake-modules/issues">
            <span class="octicon octicon-issue-opened"></span> <span class="full-word">Issues</span>
            <span class='counter'>0</span>
            <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>        </li>

      <li class="tooltipped tooltipped-w" aria-label="Pull Requests">
        <a href="/Tronic/cmake-modules/pulls" aria-label="Pull Requests" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-hotkey="g p" data-selected-links="repo_pulls /Tronic/cmake-modules/pulls">
            <span class="octicon octicon-git-pull-request"></span> <span class="full-word">Pull Requests</span>
            <span class='counter'>0</span>
            <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>


    </ul>
    <div class="sunken-menu-separator"></div>
    <ul class="sunken-menu-group">

      <li class="tooltipped tooltipped-w" aria-label="Pulse">
        <a href="/Tronic/cmake-modules/pulse" aria-label="Pulse" class="js-selected-navigation-item sunken-menu-item" data-pjax="true" data-selected-links="pulse /Tronic/cmake-modules/pulse">
          <span class="octicon octicon-pulse"></span> <span class="full-word">Pulse</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped tooltipped-w" aria-label="Graphs">
        <a href="/Tronic/cmake-modules/graphs" aria-label="Graphs" class="js-selected-navigation-item sunken-menu-item" data-pjax="true" data-selected-links="repo_graphs repo_contributors /Tronic/cmake-modules/graphs">
          <span class="octicon octicon-graph"></span> <span class="full-word">Graphs</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped tooltipped-w" aria-label="Network">
        <a href="/Tronic/cmake-modules/network" aria-label="Network" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-selected-links="repo_network /Tronic/cmake-modules/network">
          <span class="octicon octicon-repo-forked"></span> <span class="full-word">Network</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>
    </ul>


  </div>
</div>

              <div class="only-with-full-nav">
                

  

<div class="clone-url open"
  data-protocol-type="http"
  data-url="/users/set_protocol?protocol_selector=http&amp;protocol_type=clone">
  <h3><strong>HTTPS</strong> clone URL</h3>
  <div class="clone-url-box">
    <input type="text" class="clone js-url-field"
           value="https://github.com/Tronic/cmake-modules.git" readonly="readonly">
    <span class="url-box-clippy">
    <button aria-label="copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="https://github.com/Tronic/cmake-modules.git" data-copied-hint="copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>

  

<div class="clone-url "
  data-protocol-type="subversion"
  data-url="/users/set_protocol?protocol_selector=subversion&amp;protocol_type=clone">
  <h3><strong>Subversion</strong> checkout URL</h3>
  <div class="clone-url-box">
    <input type="text" class="clone js-url-field"
           value="https://github.com/Tronic/cmake-modules" readonly="readonly">
    <span class="url-box-clippy">
    <button aria-label="copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="https://github.com/Tronic/cmake-modules" data-copied-hint="copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>


<p class="clone-options">You can clone with
      <a href="#" class="js-clone-selector" data-protocol="http">HTTPS</a>
      or <a href="#" class="js-clone-selector" data-protocol="subversion">Subversion</a>.
  <span class="help tooltipped tooltipped-n" aria-label="Get help on which URL is right for you.">
    <a href="https://help.github.com/articles/which-remote-url-should-i-use">
    <span class="octicon octicon-question"></span>
    </a>
  </span>
</p>

  <a href="http://mac.github.com" data-url="github-mac://openRepo/https://github.com/Tronic/cmake-modules" class="minibutton sidebar-button js-conduit-rewrite-url" title="Save Tronic/cmake-modules to your computer and use it in GitHub Desktop." aria-label="Save Tronic/cmake-modules to your computer and use it in GitHub Desktop.">
    <span class="octicon octicon-device-desktop"></span>
    Clone in Desktop
  </a>


                <a href="/Tronic/cmake-modules/archive/master.zip"
                   class="minibutton sidebar-button"
                   aria-label="Download Tronic/cmake-modules as a zip file"
                   title="Download Tronic/cmake-modules as a zip file"
                   rel="nofollow">
                  <span class="octicon octicon-cloud-download"></span>
                  Download ZIP
                </a>
              </div>
        </div><!-- /.repository-sidebar -->

        <div id="js-repo-pjax-container" class="repository-content context-loader-container" data-pjax-container>
          


<a href="/Tronic/cmake-modules/blob/d6b5e94625d41469eaf1e2c484d7204cd263893d/LibFindMacros.cmake" class="hidden js-permalink-shortcut" data-hotkey="y">Permalink</a>

<!-- blob contrib key: blob_contributors:v21:a2461b944fd748ed71e2ea2cfaadef97 -->

<p title="This is a placeholder element" class="js-history-link-replace hidden"></p>

<a href="/Tronic/cmake-modules/find/master" data-pjax data-hotkey="t" class="js-show-file-finder" style="display:none">Show File Finder</a>

<div class="file-navigation">
  

<div class="select-menu js-menu-container js-select-menu" >
  <span class="minibutton select-menu-button js-menu-target" data-hotkey="w"
    data-master-branch="master"
    data-ref="master"
    role="button" aria-label="Switch branches or tags" tabindex="0" aria-haspopup="true">
    <span class="octicon octicon-git-branch"></span>
    <i>branch:</i>
    <span class="js-select-button">master</span>
  </span>

  <div class="select-menu-modal-holder js-menu-content js-navigation-container" data-pjax aria-hidden="true">

    <div class="select-menu-modal">
      <div class="select-menu-header">
        <span class="select-menu-title">Switch branches/tags</span>
        <span class="octicon octicon-x js-menu-close"></span>
      </div> <!-- /.select-menu-header -->

      <div class="select-menu-filters">
        <div class="select-menu-text-filter">
          <input type="text" aria-label="Filter branches/tags" id="context-commitish-filter-field" class="js-filterable-field js-navigation-enable" placeholder="Filter branches/tags">
        </div>
        <div class="select-menu-tabs">
          <ul>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="branches" class="js-select-menu-tab">Branches</a>
            </li>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="tags" class="js-select-menu-tab">Tags</a>
            </li>
          </ul>
        </div><!-- /.select-menu-tabs -->
      </div><!-- /.select-menu-filters -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="branches">

        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <div class="select-menu-item js-navigation-item selected">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/Tronic/cmake-modules/blob/master/LibFindMacros.cmake"
                 data-name="master"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text js-select-button-text css-truncate-target"
                 title="master">master</a>
            </div> <!-- /.select-menu-item -->
        </div>

          <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="tags">
        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


        </div>

        <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

    </div> <!-- /.select-menu-modal -->
  </div> <!-- /.select-menu-modal-holder -->
</div> <!-- /.select-menu -->

  <div class="breadcrumb">
    <span class='repo-root js-repo-root'><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/Tronic/cmake-modules" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">cmake-modules</span></a></span></span><span class="separator"> / </span><strong class="final-path">LibFindMacros.cmake</strong> <button aria-label="copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="LibFindMacros.cmake" data-copied-hint="copied!" type="button"><span class="octicon octicon-clippy"></span></button>
  </div>
</div>


  <div class="commit file-history-tease">
      <img alt="Tronic" class="main-avatar js-avatar" data-user="98187" height="24" src="https://avatars0.githubusercontent.com/u/98187?s=140" width="24" />
      <span class="author"><a href="/Tronic" rel="author">Tronic</a></span>
      <time datetime="2014-05-30T15:28:48+03:00" is="relative-time">May 30, 2014</time>
      <div class="commit-title">
          <a href="/Tronic/cmake-modules/commit/d6b5e94625d41469eaf1e2c484d7204cd263893d" class="message" data-pjax="true" title="Version 2.2 released. Bugfixes only.

- Various scoping errors and one copy&amp;paste mistake were causing dependency system failure.

Note: PARENT_SCOPE means *only* parent scope, not current scope.">Version 2.2 released. Bugfixes only.</a>
      </div>

    <div class="participation">
      <p class="quickstat"><a href="#blob_contributors_box" rel="facebox"><strong>1</strong>  contributor</a></p>
      
    </div>
    <div id="blob_contributors_box" style="display:none">
      <h2 class="facebox-header">Users who have contributed to this file</h2>
      <ul class="facebox-user-list">
          <li class="facebox-user-list-item">
            <img alt="Tronic" class=" js-avatar" data-user="98187" height="24" src="https://avatars0.githubusercontent.com/u/98187?s=140" width="24" />
            <a href="/Tronic">Tronic</a>
          </li>
      </ul>
    </div>
  </div>

<div class="file-box">
  <div class="file">
    <div class="meta clearfix">
      <div class="info file-name">
        <span class="icon"><b class="octicon octicon-file-text"></b></span>
        <span class="mode" title="File Mode">file</span>
        <span class="meta-divider"></span>
          <span>267 lines (245 sloc)</span>
          <span class="meta-divider"></span>
        <span>10.703 kb</span>
      </div>
      <div class="actions">
        <div class="button-group">
            <a class="minibutton tooltipped tooltipped-w js-conduit-openfile-check"
               href="http://mac.github.com"
               data-url="github-mac://openRepo/https://github.com/Tronic/cmake-modules?branch=master&amp;filepath=LibFindMacros.cmake"
               aria-label="Open this file in GitHub for Mac"
               data-failed-title="Your version of GitHub for Mac is too old to open this file. Try checking for updates.">
                <span class="octicon octicon-device-desktop"></span> Open
            </a>
              <a class="minibutton disabled tooltipped tooltipped-w" href="#"
                 aria-label="You must be signed in to make or propose changes">Edit</a>
          <a href="/Tronic/cmake-modules/raw/master/LibFindMacros.cmake" class="button minibutton " id="raw-url">Raw</a>
            <a href="/Tronic/cmake-modules/blame/master/LibFindMacros.cmake" class="button minibutton js-update-url-with-hash">Blame</a>
          <a href="/Tronic/cmake-modules/commits/master/LibFindMacros.cmake" class="button minibutton " rel="nofollow">History</a>
        </div><!-- /.button-group -->
          <a class="minibutton danger disabled empty-icon tooltipped tooltipped-w" href="#"
             aria-label="You must be signed in to make or propose changes">
          Delete
        </a>
      </div><!-- /.actions -->
    </div>
      
  <div class="blob-wrapper data type-cmake js-blob-data">
       <table class="file-code file-diff tab-size-8">
         <tr class="file-code-line">
           <td class="blob-line-nums">
             <span id="L1" rel="#L1">1</span>
<span id="L2" rel="#L2">2</span>
<span id="L3" rel="#L3">3</span>
<span id="L4" rel="#L4">4</span>
<span id="L5" rel="#L5">5</span>
<span id="L6" rel="#L6">6</span>
<span id="L7" rel="#L7">7</span>
<span id="L8" rel="#L8">8</span>
<span id="L9" rel="#L9">9</span>
<span id="L10" rel="#L10">10</span>
<span id="L11" rel="#L11">11</span>
<span id="L12" rel="#L12">12</span>
<span id="L13" rel="#L13">13</span>
<span id="L14" rel="#L14">14</span>
<span id="L15" rel="#L15">15</span>
<span id="L16" rel="#L16">16</span>
<span id="L17" rel="#L17">17</span>
<span id="L18" rel="#L18">18</span>
<span id="L19" rel="#L19">19</span>
<span id="L20" rel="#L20">20</span>
<span id="L21" rel="#L21">21</span>
<span id="L22" rel="#L22">22</span>
<span id="L23" rel="#L23">23</span>
<span id="L24" rel="#L24">24</span>
<span id="L25" rel="#L25">25</span>
<span id="L26" rel="#L26">26</span>
<span id="L27" rel="#L27">27</span>
<span id="L28" rel="#L28">28</span>
<span id="L29" rel="#L29">29</span>
<span id="L30" rel="#L30">30</span>
<span id="L31" rel="#L31">31</span>
<span id="L32" rel="#L32">32</span>
<span id="L33" rel="#L33">33</span>
<span id="L34" rel="#L34">34</span>
<span id="L35" rel="#L35">35</span>
<span id="L36" rel="#L36">36</span>
<span id="L37" rel="#L37">37</span>
<span id="L38" rel="#L38">38</span>
<span id="L39" rel="#L39">39</span>
<span id="L40" rel="#L40">40</span>
<span id="L41" rel="#L41">41</span>
<span id="L42" rel="#L42">42</span>
<span id="L43" rel="#L43">43</span>
<span id="L44" rel="#L44">44</span>
<span id="L45" rel="#L45">45</span>
<span id="L46" rel="#L46">46</span>
<span id="L47" rel="#L47">47</span>
<span id="L48" rel="#L48">48</span>
<span id="L49" rel="#L49">49</span>
<span id="L50" rel="#L50">50</span>
<span id="L51" rel="#L51">51</span>
<span id="L52" rel="#L52">52</span>
<span id="L53" rel="#L53">53</span>
<span id="L54" rel="#L54">54</span>
<span id="L55" rel="#L55">55</span>
<span id="L56" rel="#L56">56</span>
<span id="L57" rel="#L57">57</span>
<span id="L58" rel="#L58">58</span>
<span id="L59" rel="#L59">59</span>
<span id="L60" rel="#L60">60</span>
<span id="L61" rel="#L61">61</span>
<span id="L62" rel="#L62">62</span>
<span id="L63" rel="#L63">63</span>
<span id="L64" rel="#L64">64</span>
<span id="L65" rel="#L65">65</span>
<span id="L66" rel="#L66">66</span>
<span id="L67" rel="#L67">67</span>
<span id="L68" rel="#L68">68</span>
<span id="L69" rel="#L69">69</span>
<span id="L70" rel="#L70">70</span>
<span id="L71" rel="#L71">71</span>
<span id="L72" rel="#L72">72</span>
<span id="L73" rel="#L73">73</span>
<span id="L74" rel="#L74">74</span>
<span id="L75" rel="#L75">75</span>
<span id="L76" rel="#L76">76</span>
<span id="L77" rel="#L77">77</span>
<span id="L78" rel="#L78">78</span>
<span id="L79" rel="#L79">79</span>
<span id="L80" rel="#L80">80</span>
<span id="L81" rel="#L81">81</span>
<span id="L82" rel="#L82">82</span>
<span id="L83" rel="#L83">83</span>
<span id="L84" rel="#L84">84</span>
<span id="L85" rel="#L85">85</span>
<span id="L86" rel="#L86">86</span>
<span id="L87" rel="#L87">87</span>
<span id="L88" rel="#L88">88</span>
<span id="L89" rel="#L89">89</span>
<span id="L90" rel="#L90">90</span>
<span id="L91" rel="#L91">91</span>
<span id="L92" rel="#L92">92</span>
<span id="L93" rel="#L93">93</span>
<span id="L94" rel="#L94">94</span>
<span id="L95" rel="#L95">95</span>
<span id="L96" rel="#L96">96</span>
<span id="L97" rel="#L97">97</span>
<span id="L98" rel="#L98">98</span>
<span id="L99" rel="#L99">99</span>
<span id="L100" rel="#L100">100</span>
<span id="L101" rel="#L101">101</span>
<span id="L102" rel="#L102">102</span>
<span id="L103" rel="#L103">103</span>
<span id="L104" rel="#L104">104</span>
<span id="L105" rel="#L105">105</span>
<span id="L106" rel="#L106">106</span>
<span id="L107" rel="#L107">107</span>
<span id="L108" rel="#L108">108</span>
<span id="L109" rel="#L109">109</span>
<span id="L110" rel="#L110">110</span>
<span id="L111" rel="#L111">111</span>
<span id="L112" rel="#L112">112</span>
<span id="L113" rel="#L113">113</span>
<span id="L114" rel="#L114">114</span>
<span id="L115" rel="#L115">115</span>
<span id="L116" rel="#L116">116</span>
<span id="L117" rel="#L117">117</span>
<span id="L118" rel="#L118">118</span>
<span id="L119" rel="#L119">119</span>
<span id="L120" rel="#L120">120</span>
<span id="L121" rel="#L121">121</span>
<span id="L122" rel="#L122">122</span>
<span id="L123" rel="#L123">123</span>
<span id="L124" rel="#L124">124</span>
<span id="L125" rel="#L125">125</span>
<span id="L126" rel="#L126">126</span>
<span id="L127" rel="#L127">127</span>
<span id="L128" rel="#L128">128</span>
<span id="L129" rel="#L129">129</span>
<span id="L130" rel="#L130">130</span>
<span id="L131" rel="#L131">131</span>
<span id="L132" rel="#L132">132</span>
<span id="L133" rel="#L133">133</span>
<span id="L134" rel="#L134">134</span>
<span id="L135" rel="#L135">135</span>
<span id="L136" rel="#L136">136</span>
<span id="L137" rel="#L137">137</span>
<span id="L138" rel="#L138">138</span>
<span id="L139" rel="#L139">139</span>
<span id="L140" rel="#L140">140</span>
<span id="L141" rel="#L141">141</span>
<span id="L142" rel="#L142">142</span>
<span id="L143" rel="#L143">143</span>
<span id="L144" rel="#L144">144</span>
<span id="L145" rel="#L145">145</span>
<span id="L146" rel="#L146">146</span>
<span id="L147" rel="#L147">147</span>
<span id="L148" rel="#L148">148</span>
<span id="L149" rel="#L149">149</span>
<span id="L150" rel="#L150">150</span>
<span id="L151" rel="#L151">151</span>
<span id="L152" rel="#L152">152</span>
<span id="L153" rel="#L153">153</span>
<span id="L154" rel="#L154">154</span>
<span id="L155" rel="#L155">155</span>
<span id="L156" rel="#L156">156</span>
<span id="L157" rel="#L157">157</span>
<span id="L158" rel="#L158">158</span>
<span id="L159" rel="#L159">159</span>
<span id="L160" rel="#L160">160</span>
<span id="L161" rel="#L161">161</span>
<span id="L162" rel="#L162">162</span>
<span id="L163" rel="#L163">163</span>
<span id="L164" rel="#L164">164</span>
<span id="L165" rel="#L165">165</span>
<span id="L166" rel="#L166">166</span>
<span id="L167" rel="#L167">167</span>
<span id="L168" rel="#L168">168</span>
<span id="L169" rel="#L169">169</span>
<span id="L170" rel="#L170">170</span>
<span id="L171" rel="#L171">171</span>
<span id="L172" rel="#L172">172</span>
<span id="L173" rel="#L173">173</span>
<span id="L174" rel="#L174">174</span>
<span id="L175" rel="#L175">175</span>
<span id="L176" rel="#L176">176</span>
<span id="L177" rel="#L177">177</span>
<span id="L178" rel="#L178">178</span>
<span id="L179" rel="#L179">179</span>
<span id="L180" rel="#L180">180</span>
<span id="L181" rel="#L181">181</span>
<span id="L182" rel="#L182">182</span>
<span id="L183" rel="#L183">183</span>
<span id="L184" rel="#L184">184</span>
<span id="L185" rel="#L185">185</span>
<span id="L186" rel="#L186">186</span>
<span id="L187" rel="#L187">187</span>
<span id="L188" rel="#L188">188</span>
<span id="L189" rel="#L189">189</span>
<span id="L190" rel="#L190">190</span>
<span id="L191" rel="#L191">191</span>
<span id="L192" rel="#L192">192</span>
<span id="L193" rel="#L193">193</span>
<span id="L194" rel="#L194">194</span>
<span id="L195" rel="#L195">195</span>
<span id="L196" rel="#L196">196</span>
<span id="L197" rel="#L197">197</span>
<span id="L198" rel="#L198">198</span>
<span id="L199" rel="#L199">199</span>
<span id="L200" rel="#L200">200</span>
<span id="L201" rel="#L201">201</span>
<span id="L202" rel="#L202">202</span>
<span id="L203" rel="#L203">203</span>
<span id="L204" rel="#L204">204</span>
<span id="L205" rel="#L205">205</span>
<span id="L206" rel="#L206">206</span>
<span id="L207" rel="#L207">207</span>
<span id="L208" rel="#L208">208</span>
<span id="L209" rel="#L209">209</span>
<span id="L210" rel="#L210">210</span>
<span id="L211" rel="#L211">211</span>
<span id="L212" rel="#L212">212</span>
<span id="L213" rel="#L213">213</span>
<span id="L214" rel="#L214">214</span>
<span id="L215" rel="#L215">215</span>
<span id="L216" rel="#L216">216</span>
<span id="L217" rel="#L217">217</span>
<span id="L218" rel="#L218">218</span>
<span id="L219" rel="#L219">219</span>
<span id="L220" rel="#L220">220</span>
<span id="L221" rel="#L221">221</span>
<span id="L222" rel="#L222">222</span>
<span id="L223" rel="#L223">223</span>
<span id="L224" rel="#L224">224</span>
<span id="L225" rel="#L225">225</span>
<span id="L226" rel="#L226">226</span>
<span id="L227" rel="#L227">227</span>
<span id="L228" rel="#L228">228</span>
<span id="L229" rel="#L229">229</span>
<span id="L230" rel="#L230">230</span>
<span id="L231" rel="#L231">231</span>
<span id="L232" rel="#L232">232</span>
<span id="L233" rel="#L233">233</span>
<span id="L234" rel="#L234">234</span>
<span id="L235" rel="#L235">235</span>
<span id="L236" rel="#L236">236</span>
<span id="L237" rel="#L237">237</span>
<span id="L238" rel="#L238">238</span>
<span id="L239" rel="#L239">239</span>
<span id="L240" rel="#L240">240</span>
<span id="L241" rel="#L241">241</span>
<span id="L242" rel="#L242">242</span>
<span id="L243" rel="#L243">243</span>
<span id="L244" rel="#L244">244</span>
<span id="L245" rel="#L245">245</span>
<span id="L246" rel="#L246">246</span>
<span id="L247" rel="#L247">247</span>
<span id="L248" rel="#L248">248</span>
<span id="L249" rel="#L249">249</span>
<span id="L250" rel="#L250">250</span>
<span id="L251" rel="#L251">251</span>
<span id="L252" rel="#L252">252</span>
<span id="L253" rel="#L253">253</span>
<span id="L254" rel="#L254">254</span>
<span id="L255" rel="#L255">255</span>
<span id="L256" rel="#L256">256</span>
<span id="L257" rel="#L257">257</span>
<span id="L258" rel="#L258">258</span>
<span id="L259" rel="#L259">259</span>
<span id="L260" rel="#L260">260</span>
<span id="L261" rel="#L261">261</span>
<span id="L262" rel="#L262">262</span>
<span id="L263" rel="#L263">263</span>
<span id="L264" rel="#L264">264</span>
<span id="L265" rel="#L265">265</span>
<span id="L266" rel="#L266">266</span>

           </td>
           <td class="blob-line-code"><div class="code-body highlight"><pre><div class='line' id='LC1'><span class="c"># Version 2.2</span></div><div class='line' id='LC2'><span class="c"># Public Domain, originally written by Lasse Kärkkäinen &lt;tronic&gt;</span></div><div class='line' id='LC3'><span class="c"># Maintained at https://github.com/Tronic/cmake-modules</span></div><div class='line' id='LC4'><span class="c"># Please send your improvements as pull requests on Github.</span></div><div class='line' id='LC5'><br/></div><div class='line' id='LC6'><span class="c"># Find another package and make it a dependency of the current package.</span></div><div class='line' id='LC7'><span class="c"># This also automatically forwards the &quot;REQUIRED&quot; argument.</span></div><div class='line' id='LC8'><span class="c"># Usage: libfind_package(&lt;prefix&gt; &lt;another package&gt; [extra args to find_package])</span></div><div class='line' id='LC9'><span class="nb">macro</span> <span class="p">(</span><span class="s">libfind_package</span> <span class="s">PREFIX</span> <span class="s">PKG</span><span class="p">)</span></div><div class='line' id='LC10'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_args</span> <span class="o">${</span><span class="nv">PKG</span><span class="o">}</span> <span class="o">${</span><span class="nv">ARGN</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC11'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_FIND_REQUIRED</span><span class="p">)</span></div><div class='line' id='LC12'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_args</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_args}</span> <span class="s">REQUIRED</span><span class="p">)</span></div><div class='line' id='LC13'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC14'>&nbsp;&nbsp;<span class="nb">find_package</span><span class="p">(</span><span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_args}</span><span class="p">)</span></div><div class='line' id='LC15'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_DEPENDENCIES</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_DEPENDENCIES};</span><span class="o">${</span><span class="nv">PKG</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC16'>&nbsp;&nbsp;<span class="nb">unset</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_args</span><span class="p">)</span></div><div class='line' id='LC17'><span class="nb">endmacro</span><span class="p">()</span></div><div class='line' id='LC18'><br/></div><div class='line' id='LC19'><span class="c"># A simple wrapper to make pkg-config searches a bit easier.</span></div><div class='line' id='LC20'><span class="c"># Works the same as CMake&#39;s internal pkg_check_modules but is always quiet.</span></div><div class='line' id='LC21'><span class="nb">macro</span> <span class="p">(</span><span class="s">libfind_pkg_check_modules</span><span class="p">)</span></div><div class='line' id='LC22'>&nbsp;&nbsp;<span class="nb">find_package</span><span class="p">(</span><span class="s">PkgConfig</span> <span class="s">QUIET</span><span class="p">)</span></div><div class='line' id='LC23'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">PKG_CONFIG_FOUND</span><span class="p">)</span></div><div class='line' id='LC24'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">pkg_check_modules</span><span class="p">(</span><span class="o">${</span><span class="nv">ARGN</span><span class="o">}</span> <span class="s">QUIET</span><span class="p">)</span></div><div class='line' id='LC25'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC26'><span class="nb">endmacro</span><span class="p">()</span></div><div class='line' id='LC27'><br/></div><div class='line' id='LC28'><span class="c"># Avoid useless copy&amp;pasta by doing what most simple libraries do anyway:</span></div><div class='line' id='LC29'><span class="c"># pkg-config, find headers, find library.</span></div><div class='line' id='LC30'><span class="c"># Usage: libfind_pkg_detect(&lt;prefix&gt; &lt;pkg-config args&gt; FIND_PATH &lt;name&gt; [other args] FIND_LIBRARY &lt;name&gt; [other args])</span></div><div class='line' id='LC31'><span class="c"># E.g. libfind_pkg_detect(SDL2 sdl2 FIND_PATH SDL.h PATH_SUFFIXES SDL2 FIND_LIBRARY SDL2)</span></div><div class='line' id='LC32'><span class="nb">function</span> <span class="p">(</span><span class="s">libfind_pkg_detect</span> <span class="s">PREFIX</span><span class="p">)</span></div><div class='line' id='LC33'>&nbsp;&nbsp;<span class="c"># Parse arguments</span></div><div class='line' id='LC34'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">argname</span> <span class="s">pkgargs</span><span class="p">)</span></div><div class='line' id='LC35'>&nbsp;&nbsp;<span class="nb">foreach</span> <span class="p">(</span><span class="s">i</span> <span class="o">${</span><span class="nv">ARGN</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC36'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s2">&quot;${i}&quot;</span> <span class="s">STREQUAL</span> <span class="s2">&quot;FIND_PATH&quot;</span><span class="p">)</span></div><div class='line' id='LC37'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">argname</span> <span class="s">pathargs</span><span class="p">)</span></div><div class='line' id='LC38'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">elseif</span> <span class="p">(</span><span class="s2">&quot;${i}&quot;</span> <span class="s">STREQUAL</span> <span class="s2">&quot;FIND_LIBRARY&quot;</span><span class="p">)</span></div><div class='line' id='LC39'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">argname</span> <span class="s">libraryargs</span><span class="p">)</span></div><div class='line' id='LC40'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC41'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="o">${</span><span class="nv">argname</span><span class="o">}</span> <span class="o">${</span><span class="nv">${argname</span><span class="o">}</span><span class="s">}</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC42'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC43'>&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC44'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">pkgargs</span><span class="p">)</span></div><div class='line' id='LC45'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">FATAL_ERROR</span> <span class="s2">&quot;libfind_pkg_detect requires at least a pkg_config package name to be passed.&quot;</span><span class="p">)</span></div><div class='line' id='LC46'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC47'>&nbsp;&nbsp;<span class="c"># Find library</span></div><div class='line' id='LC48'>&nbsp;&nbsp;<span class="nb">libfind_pkg_check_modules</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_PKGCONF</span> <span class="o">${</span><span class="nv">pkgargs</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC49'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">pathargs</span><span class="p">)</span></div><div class='line' id='LC50'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">find_path</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_INCLUDE_DIR</span> <span class="s">NAMES</span> <span class="o">${</span><span class="nv">pathargs</span><span class="o">}</span> <span class="s">HINTS</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_PKGCONF_INCLUDE_DIRS}</span><span class="p">)</span></div><div class='line' id='LC51'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC52'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">libraryargs</span><span class="p">)</span></div><div class='line' id='LC53'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">find_library</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_LIBRARY</span> <span class="s">NAMES</span> <span class="o">${</span><span class="nv">libraryargs</span><span class="o">}</span> <span class="s">HINTS</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_PKGCONF_LIBRARY_DIRS}</span><span class="p">)</span></div><div class='line' id='LC54'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC55'><span class="nb">endfunction</span><span class="p">()</span></div><div class='line' id='LC56'><br/></div><div class='line' id='LC57'><span class="c"># Extracts a version #define from a version.h file, output stored to &lt;PREFIX&gt;_VERSION.</span></div><div class='line' id='LC58'><span class="c"># Usage: libfind_version_header(Foobar foobar/version.h FOOBAR_VERSION_STR)</span></div><div class='line' id='LC59'><span class="c"># Fourth argument &quot;QUIET&quot; may be used for silently testing different define names.</span></div><div class='line' id='LC60'><span class="c"># This function does nothing if the version variable is already defined.</span></div><div class='line' id='LC61'><span class="nb">function</span> <span class="p">(</span><span class="s">libfind_version_header</span> <span class="s">PREFIX</span> <span class="s">VERSION_H</span> <span class="s">DEFINE_NAME</span><span class="p">)</span></div><div class='line' id='LC62'>&nbsp;&nbsp;<span class="c"># Skip processing if we already have a version or if the include dir was not found</span></div><div class='line' id='LC63'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_VERSION</span> <span class="s">OR</span> <span class="s">NOT</span> <span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_INCLUDE_DIR</span><span class="p">)</span></div><div class='line' id='LC64'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">return</span><span class="p">()</span></div><div class='line' id='LC65'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC66'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">quiet</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_FIND_QUIETLY}</span><span class="p">)</span></div><div class='line' id='LC67'>&nbsp;&nbsp;<span class="c"># Process optional arguments</span></div><div class='line' id='LC68'>&nbsp;&nbsp;<span class="nb">foreach</span><span class="p">(</span><span class="s">arg</span> <span class="o">${</span><span class="nv">ARGN</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC69'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">arg</span> <span class="s">STREQUAL</span> <span class="s2">&quot;QUIET&quot;</span><span class="p">)</span></div><div class='line' id='LC70'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">quiet</span> <span class="s">TRUE</span><span class="p">)</span></div><div class='line' id='LC71'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC72'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">AUTHOR_WARNING</span> <span class="s2">&quot;Unknown argument ${arg} to libfind_version_header ignored.&quot;</span><span class="p">)</span></div><div class='line' id='LC73'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC74'>&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC75'>&nbsp;&nbsp;<span class="c"># Read the header and parse for version number</span></div><div class='line' id='LC76'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">filename</span> <span class="s2">&quot;${${PREFIX}_INCLUDE_DIR}/${VERSION_H}&quot;</span><span class="p">)</span></div><div class='line' id='LC77'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">EXISTS</span> <span class="o">${</span><span class="nv">filename</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC78'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">quiet</span><span class="p">)</span></div><div class='line' id='LC79'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">AUTHOR_WARNING</span> <span class="s2">&quot;Unable to find ${${PREFIX}_INCLUDE_DIR}/${VERSION_H}&quot;</span><span class="p">)</span></div><div class='line' id='LC80'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC81'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">return</span><span class="p">()</span></div><div class='line' id='LC82'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC83'>&nbsp;&nbsp;<span class="nb">file</span><span class="p">(</span><span class="s">READ</span> <span class="s2">&quot;${filename}&quot;</span> <span class="s">header</span><span class="p">)</span></div><div class='line' id='LC84'>&nbsp;&nbsp;<span class="nb">string</span><span class="p">(</span><span class="s">REGEX</span> <span class="s">REPLACE</span> <span class="s2">&quot;.*#[ \t]*define[ \t]*${DEFINE_NAME}[ \t]*\&quot;</span><span class="p">(</span><span class="s">[^\n]*</span><span class="p">)</span><span class="s">\&quot;.*&quot;</span> <span class="s2">&quot;\\1&quot;</span> <span class="s">match</span> <span class="s2">&quot;${header}&quot;</span><span class="p">)</span></div><div class='line' id='LC85'>&nbsp;&nbsp;<span class="c"># No regex match?</span></div><div class='line' id='LC86'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">match</span> <span class="s">STREQUAL</span> <span class="s">header</span><span class="p">)</span></div><div class='line' id='LC87'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">quiet</span><span class="p">)</span></div><div class='line' id='LC88'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">AUTHOR_WARNING</span> <span class="s2">&quot;Unable to find \#define ${DEFINE_NAME} \&quot;</span><span class="s">&lt;version&gt;\</span><span class="s2">&quot; from ${${PREFIX}_INCLUDE_DIR}/${VERSION_H}&quot;</span><span class="p">)</span></div><div class='line' id='LC89'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC90'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">return</span><span class="p">()</span></div><div class='line' id='LC91'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC92'>&nbsp;&nbsp;<span class="c"># Export the version string</span></div><div class='line' id='LC93'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_VERSION</span> <span class="s2">&quot;${match}&quot;</span> <span class="s">PARENT_SCOPE</span><span class="p">)</span></div><div class='line' id='LC94'><span class="nb">endfunction</span><span class="p">()</span></div><div class='line' id='LC95'><br/></div><div class='line' id='LC96'><span class="c"># Do the final processing once the paths have been detected.</span></div><div class='line' id='LC97'><span class="c"># If include dirs are needed, ${PREFIX}_PROCESS_INCLUDES should be set to contain</span></div><div class='line' id='LC98'><span class="c"># all the variables, each of which contain one include directory.</span></div><div class='line' id='LC99'><span class="c"># Ditto for ${PREFIX}_PROCESS_LIBS and library files.</span></div><div class='line' id='LC100'><span class="c"># Will set ${PREFIX}_FOUND, ${PREFIX}_INCLUDE_DIRS and ${PREFIX}_LIBRARIES.</span></div><div class='line' id='LC101'><span class="c"># Also handles errors in case library detection was required, etc.</span></div><div class='line' id='LC102'><span class="nb">function</span> <span class="p">(</span><span class="s">libfind_process</span> <span class="s">PREFIX</span><span class="p">)</span></div><div class='line' id='LC103'>&nbsp;&nbsp;<span class="c"># Skip processing if already processed during this configuration run</span></div><div class='line' id='LC104'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_FOUND</span><span class="p">)</span></div><div class='line' id='LC105'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">return</span><span class="p">()</span></div><div class='line' id='LC106'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC107'><br/></div><div class='line' id='LC108'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">found</span> <span class="s">TRUE</span><span class="p">)</span>  <span class="c"># Start with the assumption that the package was found</span></div><div class='line' id='LC109'><br/></div><div class='line' id='LC110'>&nbsp;&nbsp;<span class="c"># Did we find any files? Did we miss includes? These are for formatting better error messages.</span></div><div class='line' id='LC111'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">some_files</span> <span class="s">FALSE</span><span class="p">)</span></div><div class='line' id='LC112'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">missing_headers</span> <span class="s">FALSE</span><span class="p">)</span></div><div class='line' id='LC113'><br/></div><div class='line' id='LC114'>&nbsp;&nbsp;<span class="c"># Shorthands for some variables that we need often</span></div><div class='line' id='LC115'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">quiet</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_FIND_QUIETLY}</span><span class="p">)</span></div><div class='line' id='LC116'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">required</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_FIND_REQUIRED}</span><span class="p">)</span></div><div class='line' id='LC117'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">exactver</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_FIND_VERSION_EXACT}</span><span class="p">)</span></div><div class='line' id='LC118'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">findver</span> <span class="s2">&quot;${${PREFIX}_FIND_VERSION}&quot;</span><span class="p">)</span></div><div class='line' id='LC119'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">version</span> <span class="s2">&quot;${${PREFIX}_VERSION}&quot;</span><span class="p">)</span></div><div class='line' id='LC120'><br/></div><div class='line' id='LC121'>&nbsp;&nbsp;<span class="c"># Lists of config option names (all, includes, libs)</span></div><div class='line' id='LC122'>&nbsp;&nbsp;<span class="nb">unset</span><span class="p">(</span><span class="s">configopts</span><span class="p">)</span></div><div class='line' id='LC123'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">includeopts</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_PROCESS_INCLUDES}</span><span class="p">)</span></div><div class='line' id='LC124'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">libraryopts</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_PROCESS_LIBS}</span><span class="p">)</span></div><div class='line' id='LC125'><br/></div><div class='line' id='LC126'>&nbsp;&nbsp;<span class="c"># Process deps to add to </span></div><div class='line' id='LC127'>&nbsp;&nbsp;<span class="nb">foreach</span> <span class="p">(</span><span class="s">i</span> <span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span> <span class="o">${</span><span class="nv">${PREFIX</span><span class="o">}</span><span class="s">_DEPENDENCIES}</span><span class="p">)</span></div><div class='line' id='LC128'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">DEFINED</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_INCLUDE_OPTS</span> <span class="s">OR</span> <span class="s">DEFINED</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_LIBRARY_OPTS</span><span class="p">)</span></div><div class='line' id='LC129'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="c"># The package seems to export option lists that we can use, woohoo!</span></div><div class='line' id='LC130'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">includeopts</span> <span class="o">${</span><span class="nv">${i</span><span class="o">}</span><span class="s">_INCLUDE_OPTS}</span><span class="p">)</span></div><div class='line' id='LC131'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">libraryopts</span> <span class="o">${</span><span class="nv">${i</span><span class="o">}</span><span class="s">_LIBRARY_OPTS}</span><span class="p">)</span></div><div class='line' id='LC132'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC133'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="c"># If plural forms don&#39;t exist or they equal singular forms</span></div><div class='line' id='LC134'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">((</span><span class="s">NOT</span> <span class="s">DEFINED</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_INCLUDE_DIRS</span> <span class="s">AND</span> <span class="s">NOT</span> <span class="s">DEFINED</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_LIBRARIES</span><span class="p">)</span> <span class="s">OR</span></div><div class='line' id='LC135'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="p">(</span><span class="s">{i}_INCLUDE_DIR</span> <span class="s">STREQUAL</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_INCLUDE_DIRS</span> <span class="s">AND</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_LIBRARY</span> <span class="s">STREQUAL</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_LIBRARIES</span><span class="p">))</span></div><div class='line' id='LC136'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="c"># Singular forms can be used</span></div><div class='line' id='LC137'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">DEFINED</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_INCLUDE_DIR</span><span class="p">)</span></div><div class='line' id='LC138'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">includeopts</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_INCLUDE_DIR</span><span class="p">)</span></div><div class='line' id='LC139'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC140'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">DEFINED</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_LIBRARY</span><span class="p">)</span></div><div class='line' id='LC141'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">libraryopts</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="s">_LIBRARY</span><span class="p">)</span></div><div class='line' id='LC142'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC143'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC144'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="c"># Oh no, we don&#39;t know the option names</span></div><div class='line' id='LC145'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">FATAL_ERROR</span> <span class="s2">&quot;We couldn&#39;t determine config variable names for ${i} includes and libs. Aieeh!&quot;</span><span class="p">)</span></div><div class='line' id='LC146'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC147'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC148'>&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC149'>&nbsp;&nbsp;</div><div class='line' id='LC150'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">includeopts</span><span class="p">)</span></div><div class='line' id='LC151'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">REMOVE_DUPLICATES</span> <span class="s">includeopts</span><span class="p">)</span></div><div class='line' id='LC152'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC153'>&nbsp;&nbsp;</div><div class='line' id='LC154'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">libraryopts</span><span class="p">)</span></div><div class='line' id='LC155'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">REMOVE_DUPLICATES</span> <span class="s">libraryopts</span><span class="p">)</span></div><div class='line' id='LC156'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC157'><br/></div><div class='line' id='LC158'>&nbsp;&nbsp;<span class="nb">string</span><span class="p">(</span><span class="s">REGEX</span> <span class="s">REPLACE</span> <span class="s2">&quot;.*[ ;]([^ ;]*(_INCLUDE_DIRS|_LIBRARIES))&quot;</span> <span class="s2">&quot;\\1&quot;</span> <span class="s">tmp</span> <span class="s2">&quot;${includeopts} ${libraryopts}&quot;</span><span class="p">)</span></div><div class='line' id='LC159'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">tmp</span> <span class="s">STREQUAL</span> <span class="s2">&quot;${includeopts} ${libraryopts}&quot;</span><span class="p">)</span></div><div class='line' id='LC160'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">AUTHOR_WARNING</span> <span class="s2">&quot;Plural form ${tmp} found in config options of ${PREFIX}. This works as before but is now deprecated. Please only use singular forms INCLUDE_DIR and LIBRARY, and update your find scripts for LibFindMacros &gt; 2.0 automatic dependency system (most often you can simply remove the PROCESS variables entirely).&quot;</span><span class="p">)</span></div><div class='line' id='LC161'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC162'><br/></div><div class='line' id='LC163'>&nbsp;&nbsp;<span class="c"># Include/library names separated by spaces (notice: not CMake lists)</span></div><div class='line' id='LC164'>&nbsp;&nbsp;<span class="nb">unset</span><span class="p">(</span><span class="s">includes</span><span class="p">)</span></div><div class='line' id='LC165'>&nbsp;&nbsp;<span class="nb">unset</span><span class="p">(</span><span class="s">libs</span><span class="p">)</span></div><div class='line' id='LC166'><br/></div><div class='line' id='LC167'>&nbsp;&nbsp;<span class="c"># Process all includes and set found false if any are missing</span></div><div class='line' id='LC168'>&nbsp;&nbsp;<span class="nb">foreach</span> <span class="p">(</span><span class="s">i</span> <span class="o">${</span><span class="nv">includeopts</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC169'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">configopts</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC170'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s2">&quot;${${i}}&quot;</span> <span class="s">STREQUAL</span> <span class="s2">&quot;${i}-NOTFOUND&quot;</span><span class="p">)</span></div><div class='line' id='LC171'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">includes</span> <span class="s2">&quot;${${i}}&quot;</span><span class="p">)</span></div><div class='line' id='LC172'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC173'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">found</span> <span class="s">FALSE</span><span class="p">)</span></div><div class='line' id='LC174'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">missing_headers</span> <span class="s">TRUE</span><span class="p">)</span></div><div class='line' id='LC175'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC176'>&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC177'><br/></div><div class='line' id='LC178'>&nbsp;&nbsp;<span class="c"># Process all libraries and set found false if any are missing</span></div><div class='line' id='LC179'>&nbsp;&nbsp;<span class="nb">foreach</span> <span class="p">(</span><span class="s">i</span> <span class="o">${</span><span class="nv">libraryopts</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC180'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">configopts</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC181'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s2">&quot;${${i}}&quot;</span> <span class="s">STREQUAL</span> <span class="s2">&quot;${i}-NOTFOUND&quot;</span><span class="p">)</span></div><div class='line' id='LC182'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">list</span><span class="p">(</span><span class="s">APPEND</span> <span class="s">libs</span> <span class="s2">&quot;${${i}}&quot;</span><span class="p">)</span></div><div class='line' id='LC183'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC184'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="s">found</span> <span class="s">FALSE</span><span class="p">)</span></div><div class='line' id='LC185'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC186'>&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC187'><br/></div><div class='line' id='LC188'>&nbsp;&nbsp;<span class="c"># Version checks</span></div><div class='line' id='LC189'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">found</span> <span class="s">AND</span> <span class="s">findver</span><span class="p">)</span></div><div class='line' id='LC190'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">version</span><span class="p">)</span></div><div class='line' id='LC191'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">WARNING</span> <span class="s2">&quot;The find module for ${PREFIX} does not provide version information, so we&#39;ll just assume that it is OK. Please fix the module or remove package version requirements to get rid of this warning.&quot;</span><span class="p">)</span></div><div class='line' id='LC192'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">elseif</span> <span class="p">(</span><span class="s">version</span> <span class="s">VERSION_LESS</span> <span class="s">findver</span> <span class="s">OR</span> <span class="p">(</span><span class="s">exactver</span> <span class="s">AND</span> <span class="s">NOT</span> <span class="s">version</span> <span class="s">VERSION_EQUAL</span> <span class="s">findver</span><span class="p">))</span></div><div class='line' id='LC193'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">found</span> <span class="s">FALSE</span><span class="p">)</span></div><div class='line' id='LC194'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">version_unsuitable</span> <span class="s">TRUE</span><span class="p">)</span></div><div class='line' id='LC195'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC196'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC197'><br/></div><div class='line' id='LC198'>&nbsp;&nbsp;<span class="c"># If all-OK, hide all config options, export variables, print status and exit</span></div><div class='line' id='LC199'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">found</span><span class="p">)</span></div><div class='line' id='LC200'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">foreach</span> <span class="p">(</span><span class="s">i</span> <span class="o">${</span><span class="nv">configopts</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC201'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">mark_as_advanced</span><span class="p">(</span><span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC202'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC203'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">quiet</span><span class="p">)</span></div><div class='line' id='LC204'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;Found ${PREFIX} ${${PREFIX}_VERSION}&quot;</span><span class="p">)</span></div><div class='line' id='LC205'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">LIBFIND_DEBUG</span><span class="p">)</span></div><div class='line' id='LC206'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;  ${PREFIX}_DEPENDENCIES=${${PREFIX}_DEPENDENCIES}&quot;</span><span class="p">)</span></div><div class='line' id='LC207'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;  ${PREFIX}_INCLUDE_OPTS=${includeopts}&quot;</span><span class="p">)</span></div><div class='line' id='LC208'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;  ${PREFIX}_INCLUDE_DIRS=${includes}&quot;</span><span class="p">)</span></div><div class='line' id='LC209'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;  ${PREFIX}_LIBRARY_OPTS=${libraryopts}&quot;</span><span class="p">)</span></div><div class='line' id='LC210'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;  ${PREFIX}_LIBRARIES=${libs}&quot;</span><span class="p">)</span></div><div class='line' id='LC211'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC212'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_INCLUDE_OPTS</span> <span class="o">${</span><span class="nv">includeopts</span><span class="o">}</span> <span class="s">PARENT_SCOPE</span><span class="p">)</span></div><div class='line' id='LC213'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_LIBRARY_OPTS</span> <span class="o">${</span><span class="nv">libraryopts</span><span class="o">}</span> <span class="s">PARENT_SCOPE</span><span class="p">)</span></div><div class='line' id='LC214'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_INCLUDE_DIRS</span> <span class="o">${</span><span class="nv">includes</span><span class="o">}</span> <span class="s">PARENT_SCOPE</span><span class="p">)</span></div><div class='line' id='LC215'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_LIBRARIES</span> <span class="o">${</span><span class="nv">libs</span><span class="o">}</span> <span class="s">PARENT_SCOPE</span><span class="p">)</span></div><div class='line' id='LC216'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="o">${</span><span class="nv">PREFIX</span><span class="o">}</span><span class="s">_FOUND</span> <span class="s">TRUE</span> <span class="s">PARENT_SCOPE</span><span class="p">)</span></div><div class='line' id='LC217'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC218'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">return</span><span class="p">()</span>    </div><div class='line' id='LC219'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC220'><br/></div><div class='line' id='LC221'>&nbsp;&nbsp;<span class="c"># Format messages for debug info and the type of error</span></div><div class='line' id='LC222'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">vars</span> <span class="s2">&quot;Relevant CMake configuration variables:\n&quot;</span><span class="p">)</span></div><div class='line' id='LC223'>&nbsp;&nbsp;<span class="nb">foreach</span> <span class="p">(</span><span class="s">i</span> <span class="o">${</span><span class="nv">configopts</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC224'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">mark_as_advanced</span><span class="p">(</span><span class="s">CLEAR</span> <span class="o">${</span><span class="nv">i</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC225'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">val</span> <span class="o">${</span><span class="nv">${i</span><span class="o">}</span><span class="s">}</span><span class="p">)</span></div><div class='line' id='LC226'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s2">&quot;${val}&quot;</span> <span class="s">STREQUAL</span> <span class="s2">&quot;${i}-NOTFOUND&quot;</span><span class="p">)</span></div><div class='line' id='LC227'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="s">val</span> <span class="s2">&quot;&lt;not found&gt;&quot;</span><span class="p">)</span></div><div class='line' id='LC228'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">elseif</span> <span class="p">(</span><span class="s">val</span> <span class="s">AND</span> <span class="s">NOT</span> <span class="s">EXISTS</span> <span class="o">${</span><span class="nv">val</span><span class="o">}</span><span class="p">)</span></div><div class='line' id='LC229'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span> <span class="p">(</span><span class="s">val</span> <span class="s2">&quot;${val}  (does not exist)&quot;</span><span class="p">)</span></div><div class='line' id='LC230'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC231'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">some_files</span> <span class="s">TRUE</span><span class="p">)</span></div><div class='line' id='LC232'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC233'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">vars</span> <span class="s2">&quot;${vars}  ${i}=${val}\n&quot;</span><span class="p">)</span></div><div class='line' id='LC234'>&nbsp;&nbsp;<span class="nb">endforeach</span><span class="p">()</span></div><div class='line' id='LC235'>&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">vars</span> <span class="s2">&quot;${vars}You may use CMake GUI, cmake -D or ccmake to modify the values. Delete CMakeCache.txt to discard all values and force full re-detection if necessary.\n&quot;</span><span class="p">)</span></div><div class='line' id='LC236'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">version_unsuitable</span><span class="p">)</span></div><div class='line' id='LC237'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;${PREFIX} ${${PREFIX}_VERSION} was found but&quot;</span><span class="p">)</span></div><div class='line' id='LC238'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">exactver</span><span class="p">)</span></div><div class='line' id='LC239'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;${msg} only version ${findver} is acceptable.&quot;</span><span class="p">)</span></div><div class='line' id='LC240'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC241'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;${msg} version ${findver} is the minimum requirement.&quot;</span><span class="p">)</span></div><div class='line' id='LC242'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC243'>&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC244'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">missing_headers</span><span class="p">)</span></div><div class='line' id='LC245'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;We could not find development headers for ${PREFIX}. Do you have the necessary dev package installed?&quot;</span><span class="p">)</span></div><div class='line' id='LC246'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">elseif</span> <span class="p">(</span><span class="s">some_files</span><span class="p">)</span></div><div class='line' id='LC247'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;We only found some files of ${PREFIX}, not all of them. Perhaps your installation is incomplete or maybe we just didn&#39;t look in the right place?&quot;</span><span class="p">)</span></div><div class='line' id='LC248'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">if</span><span class="p">(</span><span class="s">findver</span><span class="p">)</span></div><div class='line' id='LC249'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;${msg} This could also be caused by incompatible version (if it helps, at least ${PREFIX} ${findver} should work).&quot;</span><span class="p">)</span></div><div class='line' id='LC250'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC251'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">else</span><span class="p">()</span></div><div class='line' id='LC252'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;We were unable to find package ${PREFIX}.&quot;</span><span class="p">)</span></div><div class='line' id='LC253'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC254'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC255'><br/></div><div class='line' id='LC256'>&nbsp;&nbsp;<span class="c"># Fatal error out if REQUIRED</span></div><div class='line' id='LC257'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">required</span><span class="p">)</span></div><div class='line' id='LC258'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">set</span><span class="p">(</span><span class="s">msg</span> <span class="s2">&quot;REQUIRED PACKAGE NOT FOUND\n${msg} This package is REQUIRED and you need to install it or adjust CMake configuration in order to continue building ${CMAKE_PROJECT_NAME}.&quot;</span><span class="p">)</span></div><div class='line' id='LC259'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">FATAL_ERROR</span> <span class="s2">&quot;${msg}\n${vars}&quot;</span><span class="p">)</span></div><div class='line' id='LC260'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC261'>&nbsp;&nbsp;<span class="c"># Otherwise just print a nasty warning</span></div><div class='line' id='LC262'>&nbsp;&nbsp;<span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">quiet</span><span class="p">)</span></div><div class='line' id='LC263'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="nb">message</span><span class="p">(</span><span class="s">WARNING</span> <span class="s2">&quot;WARNING: MISSING PACKAGE\n${msg} This package is NOT REQUIRED and you may ignore this warning but by doing so you may miss some functionality of ${CMAKE_PROJECT_NAME}. \n${vars}&quot;</span><span class="p">)</span></div><div class='line' id='LC264'>&nbsp;&nbsp;<span class="nb">endif</span><span class="p">()</span></div><div class='line' id='LC265'><span class="nb">endfunction</span><span class="p">()</span></div><div class='line' id='LC266'><br/></div></pre></div></td>
         </tr>
       </table>
  </div>

  </div>
</div>

<a href="#jump-to-line" rel="facebox[.linejump]" data-hotkey="l" class="js-jump-to-line" style="display:none">Jump to Line</a>
<div id="jump-to-line" style="display:none">
  <form accept-charset="UTF-8" class="js-jump-to-line-form">
    <input class="linejump-input js-jump-to-line-field" type="text" placeholder="Jump to line&hellip;" autofocus>
    <button type="submit" class="button">Go</button>
  </form>
</div>

        </div>

      </div><!-- /.repo-container -->
      <div class="modal-backdrop"></div>
    </div><!-- /.container -->
  </div><!-- /.site -->


    </div><!-- /.wrapper -->

      <div class="container">
  <div class="site-footer">
    <ul class="site-footer-links right">
      <li><a href="https://status.github.com/">Status</a></li>
      <li><a href="http://developer.github.com">API</a></li>
      <li><a href="http://training.github.com">Training</a></li>
      <li><a href="http://shop.github.com">Shop</a></li>
      <li><a href="/blog">Blog</a></li>
      <li><a href="/about">About</a></li>

    </ul>

    <a href="/">
      <span class="mega-octicon octicon-mark-github" title="GitHub"></span>
    </a>

    <ul class="site-footer-links">
      <li>&copy; 2014 <span title="0.03615s from github-fe124-cp1-prd.iad.github.net">GitHub</span>, Inc.</li>
        <li><a href="/site/terms">Terms</a></li>
        <li><a href="/site/privacy">Privacy</a></li>
        <li><a href="/security">Security</a></li>
        <li><a href="/contact">Contact</a></li>
    </ul>
  </div><!-- /.site-footer -->
</div><!-- /.container -->


    <div class="fullscreen-overlay js-fullscreen-overlay" id="fullscreen_overlay">
  <div class="fullscreen-container js-fullscreen-container">
    <div class="textarea-wrap">
      <textarea name="fullscreen-contents" id="fullscreen-contents" class="fullscreen-contents js-fullscreen-contents" placeholder="" data-suggester="fullscreen_suggester"></textarea>
    </div>
  </div>
  <div class="fullscreen-sidebar">
    <a href="#" class="exit-fullscreen js-exit-fullscreen tooltipped tooltipped-w" aria-label="Exit Zen Mode">
      <span class="mega-octicon octicon-screen-normal"></span>
    </a>
    <a href="#" class="theme-switcher js-theme-switcher tooltipped tooltipped-w"
      aria-label="Switch themes">
      <span class="octicon octicon-color-mode"></span>
    </a>
  </div>
</div>



    <div id="ajax-error-message" class="flash flash-error">
      <span class="octicon octicon-alert"></span>
      <a href="#" class="octicon octicon-x close js-ajax-error-dismiss"></a>
      Something went wrong with that request. Please try again.
    </div>


      <script crossorigin="anonymous" src="https://assets-cdn.github.com/assets/frameworks-31cf39cf6a61d4c498cba6c0e9c100fb2b06b2f8.js" type="text/javascript"></script>
      <script async="async" crossorigin="anonymous" src="https://assets-cdn.github.com/assets/github-3ce38bb3847507f17846ba0a46d52f1b39114512.js" type="text/javascript"></script>
      
      
        <script async src="https://www.google-analytics.com/analytics.js"></script>
  </body>
</html>

