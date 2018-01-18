var cdnjsURL = 'https://cdnjs.cloudflare.com/ajax/libs/',
    $,
    actions = ['CUSTOM...', 'Noop', 'Left', 'Right', 'Up', 'Down', 'Select', 'Enter', 'PageUp', 'PageDown', 'Highlight', 'ParentDir', 'PreviousMenu', 'Back', 'Info', 'Pause', 'Stop', 'SkipNext', 'SkipPrevious', 'FullScreen', 'togglefullscreen', 'AspectRatio', 'StepForward', 'StepBack', 'BigStepForward', 'BigStepBack', 'SmallStepBack', 'Seek', 'ChapterOrBigStepForward', 'ChapterOrBigStepBack', 'NextScene', 'PreviousScene', 'OSD', 'PlayDVD', 'ShowVideoMenu', 'ShowSubtitles', 'NextSubtitle', 'SubtitleShiftUp', 'SubtitleShiftDown', 'SubtitleAlign', 'CodecInfo', 'NextPicture', 'PreviousPicture', 'ZoomOut', 'ZoomIn', 'IncreasePAR', 'DecreasePAR', 'Queue', 'Filter', 'Playlist', 'ZoomNormal', 'ZoomLevel1', 'ZoomLevel2', 'ZoomLevel3', 'ZoomLevel4', 'ZoomLevel5', 'ZoomLevel6', 'ZoomLevel7', 'ZoomLevel8', 'ZoomLevel9', 'NextCalibration', 'ResetCalibration', 'AnalogMove', 'Rotate', 'rotateccw', 'Close', 'subtitledelay', 'SubtitleDelayMinus', 'SubtitleDelayPlus', 'audiodelay', 'AudioDelayMinus', 'AudioDelayPlus', 'AudioNextLanguage', 'NextResolution', 'Number0', 'Number1', 'Number2', 'Number3', 'Number4', 'Number5', 'Number6', 'Number7', 'Number8', 'Number9', 'FastForward', 'Rewind', 'Play', 'PlayPause', 'Delete', 'Copy', 'Move', 'Rename', 'HideSubmenu', 'Screenshot', 'ShutDown()', 'VolumeUp', 'VolumeDown', 'Mute', 'volampup', 'volampdown', 'audiotoggledigital', 'BackSpace', 'ScrollUp', 'ScrollDown', 'AnalogFastForward', 'AnalogRewind', 'AnalogSeekForward', 'AnalogSeekBack', 'MoveItemUp', 'MoveItemDown', 'Menu', 'ContextMenu', 'Shift', 'Symbols', 'CursorLeft', 'CursorRight', 'ShowTime', 'visualisationpresetlist', 'ShowPreset', 'NextPreset', 'PreviousPreset', 'LockPreset', 'RandomPreset', 'IncreaseRating', 'DecreaseRating', 'ToggleWatched', 'NextLetter', 'PrevLetter', 'JumpSMS2', 'JumpSMS3', 'JumpSMS4', 'JumpSMS5', 'JumpSMS6', 'JumpSMS7', 'JumpSMS8', 'JumpSMS9', 'FilterSMS2', 'FilterSMS3', 'FilterSMS4', 'FilterSMS5', 'FilterSMS6', 'FilterSMS7', 'FilterSMS8', 'FilterSMS9', 'verticalshiftup', 'verticalshiftdown', 'scanitem', 'reloadkeymaps', 'increasevisrating', 'decreasevisrating', 'firstpage', 'lastpage', 'guiprofile', 'red', 'green', 'yellow', 'blue', 'CreateBookmark', 'CreatEpisodeBookmark', 'NextChannelGroup', 'PreviousChannelGroup', 'ChannelUp', 'ChannelDown', 'PlayPvr', 'PlayPvrTV', 'PlayPvrRadio', 'Record', 'StereoMode', 'ToggleStereoMode', 'SwitchPlayer', 'UpdateLibrary(video)', 'SetRatring'];
(function()
{
    // Load the script
    var script = document.createElement('script');
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
    var $inputAction = $('<input type="text">').addClass('form-control').on('change', function()
        {
            var input = this;
            $(input).prop('disabled', true).closest('.row').find('.delaction').prop('disabled', true);
            $.get('/change-' + $(input).closest('.row').attr('id') + ':' + $(input).val(), function()
            {
                $(input).prop('disabled', false).closest('.row').find('.delaction').prop('disabled', false);
            });
        }),
        $selectAction = $('<select>').addClass('selectaction form-control').on('change', function()
        {
            var select = this;
            if($(select).val() == 'CUSTOM...')
            {
                var input = $inputAction.clone(true, true);
                $(select).after(input).remove();
                $(input).focus();
            }
            else
            {
                $(select).prop('disabled', true).closest('.row').find('.delaction').prop('disabled', true);
                $.get('/change-' + $(select).closest('.row').attr('id') + ':' + $(select).val(), function()
                {
                    $(select).prop('disabled', false).closest('.row').find('.delaction').prop('disabled', false);
                });
            }
        }),
        $delaction = $('<button>').addClass('delaction btn material-icons').text('delete').on('click', function()
        {
            var button = this;
            $(button).prop('disabled', true).closest('.row').find('.selectaction').prop('disabled', true);
            $.get('/del-' + $(button).closest('.row').attr('id'), function()
            {
                $(button).closest('.row').remove();
            });
        });
    
    $.fn.extend({ insertIRCommandRowBefore: function(irCode, selectedValue)
    {
        return this.each(function()
        {
            var $actionElement;
            if(actions.indexOf(selectedValue) >= 0)
            {
                $actionElement = $selectAction.clone(true, true);
            }
            else
            {
                $actionElement = $inputAction.clone(true, true);
            }
            $('<div>').attr({ 'id': irCode }).addClass('row justify-content-center')
                .append($('<div>').addClass('col col-md-auto font-weight-bold').text(irCode))
                .append($('<div>').addClass('col col-md-auto').append($actionElement.val(selectedValue)))
                .append($('<div>').addClass('col col-md-auto').append($delaction.clone(true, true)))
                .insertBefore(this);
        });
    }, pipLoader: function()
    {
        var buttonEl = this;
        return setInterval(function()
        {
            var sec = 0;
            $(buttonEl).text('picture_in_picture' + (sec % 2 ? '_alt' : '')).css({ 'transform': 'scaleX(' + (Math.floor(sec/2)*-2+1) + ')' });
            sec ++
            if(sec > 3)
            {
                sec = 0;
            }
        }, 500);
    } });
    
    for(var i in actions)
    {
        $selectAction.append($('<option>').text(actions[i]).val(actions[i]));
    }
    $('head')
        .append($('<meta>').attr({ 'name': 'viewport', 'content': 'width=device-width, initial-scale=1, shrink-to-fit=no' }))
        .append($('<link>').attr({ 'rel': 'stylesheet', 'href': 'https://unpkg.com/bootstrap-material-design@4.0.0-beta.4/dist/css/bootstrap-material-design.min.css' }))
        .append($('<link>').attr({ 'rel': 'stylesheet', 'href': 'https://fonts.googleapis.com/css?family=Roboto:300,400,500,700|Material+Icons' }))
        .append($('<script>').attr({ 'srç': 'https://unpkg.com/bootstrap-material-design@4.0.0-beta.4/dist/js/bootstrap-material-design.js' }));
    
    $('<div>').addClass('container').appendTo('body').append($('<div>').addClass('main row justify-content-center'));
    $('.main.row')
        .append($('<div>').addClass('col col-md-auto'))
        .append($('<div>').addClass('col col-md-auto'))
        .append($('<div>').addClass('col col-md-auto'));
    var initLoadInterval = $('<button>').text('add_box').addClass('btn material-icons').on('click', function()
    {
        $(this).prop('disabled', true);
        $('.container .col.font-weight-bold').removeClass('font-weight-bold')
        var interval = $(this).pipLoader(),
            button = this;
        $.get('/listen', function(data)
        {
            clearInterval(interval);
            $(button).prop('disabled', false).text('add_box').css({ 'transform': 'none' });
            if(data.trim().length > 0)
            {
                if($('.container .row').filter('#' + data.trim()).length > 0)
                {
                    $('.container .row').filter('#' + data.trim()).find('.col:first-child').addClass('font-weight-bold');
                }
                else
                {
                    $('.main.row').insertIRCommandRowBefore(data.trim(), 'Noop');
                    $('.row.main').prev('.row').find('select').focus();
                }
            }
        });
    }).appendTo('.main .col:first-child').prop('disabled', true).pipLoader();
    $.getJSON('/getall', function(data)
    {
        $('.main.row button').text('add_box').prop('disabled', false);
        clearInterval(initLoadInterval);
        for(var i=0; i<data.length; i++)
        {
            $('.main.row').insertIRCommandRowBefore(data[i].ir, data[i].cmd);
        }
    });
}
