var $;
(function()
{
    // Load the script
    var script = document.createElement("script");
    script.src = 'https://cdnjs.cloudflare.com/ajax/libs/jquery/3.2.1/jquery.min.js';
    script.type = 'text/javascript';
    script.onload = function()
    {
        $ = window.jQuery;
        init();
    };
    document.getElementsByTagName('head')[0].appendChild(script);
})();

function init()
{
    $('<button>').text('Listen...').appendTo('body').on('click', function()
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
    });
}
