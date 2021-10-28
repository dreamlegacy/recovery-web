Content-Type: text/html;charset=utf-8
<% source include/environment.inc %>
<!DOCTYPE html>
<html>
 <head>
  <title>Dreambox <% printf $product %> - <% printf $page_title %></title>
  <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style type="text/css">
    .parallax-soc {
        background: url("images/<% printf $logo_soc %>") no-repeat center center fixed;
        min-height: <% printf $logo_height %>;
    }
    .parallax-mmc {
        background: url("images/<% printf $logo_mmc %>") no-repeat center center fixed;
        min-height: <% printf $logo_height %>;
    }
  </style>
  <style type="text/css">
   @import url("css/bulma.min.css");
   @import url("css/materialdesignicons.min.css");
   @import url("css/dreambox.css");
  </style>
  <script type="text/javascript" src="js/jquery.min.js"></script>
 </head>
 <body>
    <nav class="navbar is-fixed-top is-transparent">
        <div class="navbar-brand">
            <a class="navbar-item" href="index.dhtml">
                <img src="images/dreambox.png" alt="dreambox">
            </a>
        </div>
    </nav>
