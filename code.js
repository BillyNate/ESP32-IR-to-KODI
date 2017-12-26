var cdnjsURL = 'https://cdnjs.cloudflare.com/ajax/libs/',
    $;
(function()
{
    // Load the script
    var script = document.createElement("script");
    script.src = cdnjsURL + 'jquery/3.2.1/jquery.min.js';
    script.onload = function()
    {
        $ = window.jQuery;
        init();
    };
    document.getElementsByTagName('head')[0].appendChild(script);
})();

function init()
{
    $('head')
        .append($('<meta>').attr({ 'name': 'viewport', 'content': 'width=device-width, initial-scale=1, shrink-to-fit=no' }))
        .append($('<link>').attr({ 'rel': 'stylesheet', 'href': 'https://fonts.googleapis.com/css?family=Roboto:300,400,500,700|Material+Icons' }))
        .append($('<link>').attr({ 'rel': 'stylesheet', 'href': cdnjsURL + 'bootstrap-material-design/4.0.2/bootstrap-material-design.min.css' }))
        .append($('<script>').attr({ 'srç': cdnjsURL + 'bootstrap-material-design/4.0.2/bootstrap-material-design.umd.min.js' }));
    
    $('<div>').addClass('container').appendTo('body').append($('<div>').addClass('main row justify-content-center'));
    $('.main.row')
        .append($('<div>').addClass('col col-md-auto'))
        .append($('<div>').addClass('col col-md-auto'));
    $('<button>').text('Listen...').addClass('btn').on('click', function()
    {
        var button = this,
            sec = 1,
            interval = setInterval(function()
            {
                $(button).text('Listen' + Array(sec+1).join('.'));
                sec ++
                if(sec > 3)
                {
                    sec = 1;
                }
            }, 500);
        $.get('/listen', function(data)
        {
            clearInterval(interval);
            $(button).text('Listen...');
            $('<p>').text(data).appendTo('body');
        });
    }).appendTo('.main .col:first-child');
}
