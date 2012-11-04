define(['jquery', 'utils', 'storyjs-embed'], function($, utils) {
    function widget(id, url) {
        var parsedData = [];

        parseRSS(url, function(data) {
            constructTimeline(id, convert(data));
        });
    }

    function parseRSS(url, callback) {
        $.ajax({
            url: document.location.protocol + '//ajax.googleapis.com/ajax/services/feed/load?v=1.0&num=100000&callback=?&q=' + encodeURIComponent(url),
            dataType: 'json',
            success: function(data) {
                callback(data.responseData? data.responseData.feed: {author: null, entries: []});
            }
        });
    }

    function convert(data) {
        var dates = data.entries.map(function(v) {
            var date = new Date(v.publishedDate);
            var d = date.getFullYear() + ',' + (date.getMonth() + 1) + ',' + date.getDate();
            var media = '';
            var content = v.content;

            var $content = $('<div/>').html(content);
            $('a[href*="youtube"]', $content).replaceWith(function(i, v) {
                var parts = v.split('youtube.com/watch?');

                if(parts.length > 1) {
                    parts = parts[parts.length - 1].split('=');
                    media = 'http://youtu.be/' + parts[parts.length - 1];
                }
            });
            content = $content.html().replace('<br>www.youtube.com<br>', '', 'g');

            return {
                startDate: d,
                endData: d,
                headline: decode(v.title) || 'No Title',
                text: content,
                asset: {
                    media: media,
                    credit: '',
                    caption: ''
                }
            };
        });

        return {
            timeline: {
                headline: 'Elovalo News',
                type: 'default',
                startDate: dates[dates.length - 1].startDate,
                //text: "<p>Intro body text goes here, some HTML is ok</p>p>",
                date: dates
            }
        };
    }

    function decode(t) {
        return $('<div/>').html(t).text();
    }

    function constructTimeline(id, data) {
        createStoryJS({
            type: 'timeline',
            width: '100%',
            height: '600',
            source: data,
            embed_id: id,
            start_at_end: true,
            debug: true
        });
    }

    return {
        widget: widget
    };
});
