<!DOCTYPE html>
<html lang="en">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <!-- Meta, title, CSS, favicons, etc. -->
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
	<meta name="apple-mobile-web-app-capable" content="yes">
	<link rel="shortcut icon" href="image/icon.png">
	<link rel="apple-touch-icon" href="image/icon.png">
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>XLNT</title>

    <!-- Bootstrap -->
    <link href="./vendors/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">
    <!-- Font Awesome -->
    <link href="./vendors/font-awesome/css/font-awesome.min.css" rel="stylesheet">
    <!-- NProgress -->
    <link href="./vendors/nprogress/nprogress.css" rel="stylesheet">
    <!-- iCheck -->
    <link href="./vendors/iCheck/skins/flat/green.css" rel="stylesheet">
	
    <!-- bootstrap-progressbar -->
    <link href="./vendors/bootstrap-progressbar/css/bootstrap-progressbar-3.3.4.min.css" rel="stylesheet">
    <!-- JQVMap 
    <link href="./vendors/jqvmap/dist/jqvmap.min.css" rel="stylesheet"/>-->
    <!-- bootstrap-daterangepicker -->
    <link href="./vendors/bootstrap-daterangepicker/daterangepicker.css" rel="stylesheet">

    <!-- Custom Theme Style -->
    <link href="./build/css/custom.min.css" rel="stylesheet">
	<script type="text/javascript" src="js/jquery-3.2.1.min.js"></script>
	
<script type="text/javascript">
	$(document).ready(function() {
		isMobileDevice = navigator.userAgent.match(/Android|BlackBerry|Windows Phone|iPad|iPhone|iPod/i) != null 
		|| screen.width <= 480;
		screen_width = 0;//screen.width;
		//if (isMobileDevice) alert('isMobileDevice');
		chart_height = 0;
		sec = 0;
		noidung_display = false;
		Online_display = true;
		chart_display = true;
		
		if (!isMobileDevice)
		{
			chart_height = 400;
			$('#top_title').html('<h3>TRẠM QUAN TRẮC NHÀ MÁY XLNT ĐNĐN QUẢNG NAM</h3>');
		}
		else
		{
			chart_height = 0;
			$('#top_title').html('<h3>TRẠM QUAN TRẮC XLNT</h3>');
			document.getElementById("logo").innerHTML = '<img width="50%" src="image/novas.png" align="right" alt="logo">';
		}
		key_value =  "Chart";//document.getElementById("home").text;//$("#home").val();	
		channel = 'CH1';
		$.get('listview.php',{key:'Channel'},function(data){
			$('#chart_menu').html(data);
			});
		getconfig();
		showchart();
        getData();
		hidedatetime(true);
		
		$("#reload").click(function(){
			location.reload();
			// key_value =  $("#home").val();
			// showchart();
			// getData();
			// toggle_menu();
			});
		$("#home").click(function(){
			key_value =  "Chart";//$("#home").val();
			showchart();
			hidedatetime(true);
			getData();
			drawChart();
			toggle_menu();
			});
		//child_menu
		// $("#home").click(function(){
			// key_value =  $("#home").val();
			// showchart();
			// getData();
			// toggle_menu();
			// collapse_menu();
			// });		
		// $("#History_Alarm").click(function(){
			// key_value = document.getElementById("History_Alarm").text; //$("#History_Alarm").val();
			// getconfig();
			// hidechart();
			// getData();		
			// toggle_menu();			
			// collapse_menu();
			// });
		// $("#History_Data").click(function(){
			// key_value = document.getElementById("History_Data").text; //$("#History_Data").val();
			// getconfig();
			// hidechart();
			// getData();
			// toggle_menu();
			// collapse_menu();
			// });
		// $("#History_ftp").click(function(){
			// key_value = document.getElementById("History_ftp").text; //$("#History_ftp").val();
			// getconfig();
			// hidechart();
			// getData();
			// toggle_menu();
			// collapse_menu();
			// });
		// $("#History_sms").click(function(){
			// key_value = document.getElementById("History_sms").text; //$("#History_sms").val();
			// getconfig();
			// hidechart();
			// getData();
			// toggle_menu();
			// collapse_menu();
			// });			
		//child_menu1
		$("#dt_from").change(function(){
			getconfig();
			getData();
			drawChart();
			});
		$("#dt_to").change(function(){
			getconfig();
			getData();
			drawChart();
			});
		$(function(){$(window).on("resize",function(a){
				// if (screen_width != screen.width)
				// {
					// screen_width = screen.width;
					// drawChart();
				// }
				changedevice();
				drawChart();
			});
		});
    });
</script>
<script language="javascript" type="text/javascript">

function getconfig()
{
	dt_from = $("#dt_from").val();
	dt_to = $("#dt_to").val();
	//alert(dt_from);
	document.getElementById("toexcell").href="toexcell.php?key="+key_value+"&from="+dt_from+"&to="+dt_to;
	document.getElementById("tocsv").href="tocsv.php?key="+key_value+"&from="+dt_from+"&to="+dt_to;
}
function collapse_menu()
{
	if (!isMobileDevice) $("#child_menu").trigger('click');
}
function collapse_chartmenu()
{
	if (!isMobileDevice) $("#chart_menu").trigger('click');
}
function toggle_menu()
{
	//var w = document.getElementById("sidemenu").tog;
	//alert(w);
	if (isMobileDevice) $("#menu_toggle").trigger('click');
}
function hidechart()
{
	chart_display = false;
	$("#trend").hide();
	$("#dt").show();
	$("#noidung").show(); noidung_display = true;
	if (isMobileDevice) {$('#update_data').hide(); Online_display = false;}
	document.getElementById('key_selected').innerHTML = key_value;
}
function hidedatetime(hide)
{
	if (hide) $("#dt").hide();
	else $("#dt").show();
}
function showchart()
{
	chart_display = true;
	$("#trend").show();
	$("#noidung").hide();
	noidung_display = false;
	//$("#dt").hide();
	$('#update_data').show();
	Online_display = true;
	document.getElementById('key_selected').innerHTML = key_value;
}
function data_click(val)
{
	key_value = val;
	hidechart();
	getData();
	toggle_menu();
	collapse_menu();
	drawChart();
}
function chart_click(ch)
{
	//alert(ch);
	key_value = ch + " Chart";
	channel = ch;
	showchart();
	if (isMobileDevice) {$('#update_data').hide();Online_display = false;}
	hidedatetime(false);
	getconfig();
	drawChart();
	toggle_menu();
	//collapse_chartmenu();
}
function getData()
{
	if (Online_display){
	$.get('listview.php',{key:'Online',from:dt_from,to:dt_to},function(data){
			$('#update_data').html(data);
			});
	}
	if (noidung_display){
	$.get('listview.php',{key:key_value,from:dt_from,to:dt_to},function(data){
			$('#noidung').html(data);
			});
	}
}
function drawChart()
{
	if (chart_display) drawChart_1();
	sec = 0;
}
setInterval(function(){
      getData();
	  sec += 10;
	  if (sec>=60) drawChart();
 },10000);

</script>

<script type="text/javascript" src="js/loader.js"></script>
    <script type="text/javascript">
    
    // Load the Visualization API and the piechart package.
	//google.charts.load('current', {packages: ['corechart', 'line']});
	google.charts.load('current', {'packages':['corechart']});
    google.charts.setOnLoadCallback(drawChart_1);
	// google.charts.setOnLoadCallback(drawChart_2);
	// google.charts.setOnLoadCallback(drawChart_3);
	// google.charts.setOnLoadCallback(drawChart_4);
      
    function drawChart_1() {
      $.get('getJson.php',{from:dt_from,to:dt_to,column:channel},function(data){
		//alert(data);
      // Create our data table out of JSON data loaded from server.
      var data = new google.visualization.DataTable(data);//
		var options = {
				height:chart_height,
			//title: 'History COD',
				hAxis: {
				  title: 'DateTime'
				},
			legend: { position: 'top' },
			// curveType: 'function',
			series: {
				0: { color: '#01DF01' }, //#FF0080 #FF8000
				1: { color: '#FF8000' }, //Min
				2: { color: '#FF0080' }, //Max
				},
			// vAxis: {
				// minValue: 0, 
				// maxValue: 140
				// },
			  };
      // Instantiate and draw our chart, passing in some options.
      var chart = new google.visualization.LineChart(document.getElementById('chart_div_1'));
      chart.draw(data, options);
	  });
    }
 </script>
  </head>

  <body class="nav-md" style="background-color: #F7F7F7;">
    <div class="container body">
      <div class="main_container">
        <div id = "sidemenu" class="col-md-3 left_col">
          <div class="left_col scroll-view">
            <div class="navbar nav_title" style="border: 0;">
              <a id = "reload" class="site_title"><i class="fa fa-refresh"></i> <span>NM XLNT</span></a>
            </div>

            <div class="clearfix"></div>

            <!-- menu profile quick info  -->
            <div class="profile clearfix">
              <div class="profile_pic">
                <img src="image/icon.png" alt="..." class="img-circle profile_img">
              </div>
              <div class="profile_info"> 
				<span>KCN ĐNĐN QN</span>
				<h2>TRẠM QUAN TRẮC</h2>
              </div>
            </div>
            <!-- /menu profile quick info -->

            <br />

            <!-- sidebar menu -->
            <div id="sidebar-menu" class="main_menu_side hidden-print main_menu">
              <div class="menu_section">
                
                <ul id = "nav_side_menu" class="nav side-menu">
                  
                  <li><a id = "home" ><i class="fa fa-home"></i>Home</a></li>
                  <li><a id = "child_menu"><i class="fa fa-folder-open"></i> History <span class="fa fa-chevron-down"></span></a>
                    <ul class="nav child_menu">
					  <li><a id = "History_Data" href="#" onclick="data_click(this.text);return false;">History Data</a></li>                      
					  <li><a id = "History_Alarm" href="#" onclick="data_click(this.text);return false;">History Alarm</a></li>		  
					  <li><a id = "History_sms" href="#" onclick="data_click(this.text);return false;">History SMS</a></li>
					  <li><a id = "History_ftp" href="#" onclick="data_click(this.text);return false;">History Send Report</a></li>
                    </ul>
                  </li>
				  <li><a id = "chart_menu">
					<i class="fa fa-line-chart"></i> Chart <span class="fa fa-chevron-down"></span></a>
                    <ul class="nav child_menu">
					  <!--<li><a id = "CH1" href="#" onclick="chart_click(this.text);return false;">COD</a></li> -->
                    </ul>
                  </li>
                  <li><a><i class="fa fa-file"></i> Export <span class="fa fa-chevron-down"></span></a>
                    <ul class="nav child_menu">
					  <li><a id = "tocsv" target="_blank">Export To csv</a></li>
                      <li><a id = "toexcell" target="_blank">Export To excell</a></li>
                    </ul>
                  </li>
                </ul>
              </div>
              

            </div>
            <!-- /sidebar menu -->

            <!-- /menu footer buttons -->
            <div class="sidebar-footer hidden-small">
              <a data-toggle="tooltip" data-placement="top" title="Settings">
                <span class="glyphicon glyphicon-cog" aria-hidden="true"></span>
              </a>
              <a data-toggle="tooltip" data-placement="top" title="FullScreen">
                <span class="glyphicon glyphicon-fullscreen" aria-hidden="true"></span>
              </a>
              <a data-toggle="tooltip" data-placement="top" title="Lock">
                <span class="glyphicon glyphicon-eye-close" aria-hidden="true"></span>
              </a>
              <a data-toggle="tooltip" data-placement="top" title="Logout" href="#">
                <span class="glyphicon glyphicon-off" aria-hidden="true"></span>
              </a>
            </div>
            <!-- /menu footer buttons -->
          </div>
        </div>

        <!-- top navigation -->
        <div class="top_nav">
          <div class="nav_menu">
            <nav>
              <div class="nav toggle">
                <a id="menu_toggle"><i class="fa fa-bars"></i></a>
				
              </div>
				<ul class="nav navbar-nav navbar-right">
                <li id = "logo" class="">
                    </h2><img width="70%" src="image/novas.png" align="right" alt="logo">
				</li>
				</ul>
            </nav>
          </div>
        </div>
        <!-- /top navigation -->

        <!-- page content -->
        <div class="right_col" role="main">
		<div id = "top_title" style="color: #088A4B; background-color:#F7F8E0; font-style: normal; font-size: 14px;">
			<h3>xx</h3>
		</div>
          <!-- top tiles -->
          <div id = "update_data" class="row tile_count">
			
			<div class="col-md-2 col-sm-4 col-xs-6 tile_stats_count">
              <span class="count_top"><i class="fa fa-user"></i> COD</span>
              <div class="count"></div>
              <span class="count_bottom">...</span>
            </div>
            <div class="col-md-2 col-sm-4 col-xs-6 tile_stats_count">
              <span class="count_top"><i class="fa fa-clock-o"></i> pH</span>
              <div class="count"></div>
              <span class="count_bottom"><i class="green">...</span>
            </div>
			
          </div>
          <!-- /top tiles -->	
          
		<div class="row" id = "dt" style="color: #088A4B; font-style: normal; font-size: 14px;">
		<?php
			date_default_timezone_set("Asia/Ho_Chi_Minh");
			$time = time() ;//+ 25200;
			$to = date('Y-m-d', $time);
			$from = $to;//date('Y-m-d', $time-2592000);
		?>
		
        <label id = "key_selected" style="font-size: 18px;">Report</label>
          :
        <input id = "dt_from" type="date" name="from" value="<?php echo $from;?>">
        -
        <input id = "dt_to" type="date" name="to" value="<?php echo $to;?>">
        </div>    
		<div id = "trend" class="row">
			<div id = "chart_div_1"> </div>
			
          </div>
         </br> <!--  -->	
		<div class="clearfix"></div> 
        <div  id = "noidung" class="table table-striped" style="color: #0B3861; font-style: normal; font-size: 14px; background-color: #ffffff;"> 
    
    	</div>
		<div class="clearfix"></div>

          <div class="row">
            <div class="col-md-4 col-sm-4 col-xs-12"></div>


            <div class="col-md-8 col-sm-8 col-xs-12">



              <div class="row">

                <div class="col-md-12 col-sm-12 col-xs-12"></div>

              </div>
              <div class="row">


                <!-- Start to do list -->
                <div class="col-md-6 col-sm-6 col-xs-12"></div>
                <!-- End to do list -->
                
                <!-- start of weather widget -->
                <div class="col-md-6 col-sm-6 col-xs-12"></div>
                <!-- end of weather widget -->
              </div>
            </div>
          </div>
        </div>
        <!-- /page content -->

        <!-- footer content -->
        <footer>
          <div class="pull-right">
            Novas technology co.,ltd;  website:<a href="http://novas.com.vn">novas.com.vn</a>
          </div>
          <div class="clearfix"></div>
        </footer>
        <!-- /footer content -->
      </div>
    </div>

    <!-- jQuery -->
    <script src="./vendors/jquery/dist/jquery.min.js"></script>
    <!-- Bootstrap -->
    <script src="./vendors/bootstrap/dist/js/bootstrap.min.js"></script>
    <!-- FastClick -->
    <script src="./vendors/fastclick/lib/fastclick.js"></script>
    <!-- NProgress -->
    <script src="./vendors/nprogress/nprogress.js"></script>
    <!-- Chart.js -->
    <script src="./vendors/Chart.js/dist/Chart.min.js"></script>
    <!-- gauge.js -->
    <script src="./vendors/gauge.js/dist/gauge.min.js"></script>
    <!-- bootstrap-progressbar -->
    <script src="./vendors/bootstrap-progressbar/bootstrap-progressbar.min.js"></script>
    <!-- iCheck -->
    <script src="./vendors/iCheck/icheck.min.js"></script>
    <!-- Skycons -->
    <script src="./vendors/skycons/skycons.js"></script>
    <!-- Flot 
    <script src="./vendors/Flot/jquery.flot.js"></script>
    <script src="./vendors/Flot/jquery.flot.pie.js"></script>
    <script src="./vendors/Flot/jquery.flot.time.js"></script>
    <script src="./vendors/Flot/jquery.flot.stack.js"></script>
    <script src="./vendors/Flot/jquery.flot.resize.js"></script>-->
    <!-- Flot plugins 
    <script src="./vendors/flot.orderbars/js/jquery.flot.orderBars.js"></script>
    <script src="./vendors/flot-spline/js/jquery.flot.spline.min.js"></script>
    <script src="./vendors/flot.curvedlines/curvedLines.js"></script>-->
    <!-- DateJS 
    <script src="./vendors/DateJS/build/date.js"></script>-->
    <!-- JQVMap 
    <script src="./vendors/jqvmap/dist/jquery.vmap.js"></script>
    <script src="./vendors/jqvmap/dist/maps/jquery.vmap.world.js"></script>
    <script src="./vendors/jqvmap/examples/js/jquery.vmap.sampledata.js"></script>-->
    <!-- bootstrap-daterangepicker -->
    <script src="./vendors/moment/min/moment.min.js"></script>
    <script src="./vendors/bootstrap-daterangepicker/daterangepicker.js"></script>

    <!-- Custom Theme Scripts -->
    <script src="./build/js/custom.min.js"></script>
	
  </body>
</html>
