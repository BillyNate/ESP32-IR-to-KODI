var $;
(function() {
    // Load the script
    var script = document.createElement("SCRIPT");
    script.src = 'https://cdnjs.cloudflare.com/ajax/libs/jquery/3.2.1/jquery.min.js';
    script.type = 'text/javascript';
    script.onload = function() {
        $ = window.jQuery;
        init();
    };
    document.getElementsByTagName('head')[0].appendChild(script);
})();

function init()
{
    $('body').css({ 'background-color': '#00f' });
}
