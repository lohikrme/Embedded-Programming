const url =
    'https://api.thingspeak.com/channels/3101616/feeds.json?api_key=G21S5PLQ88TBL6ZB&results=20';

fetch(url)
    .then((response) => response.json())
    .then((data) => {
        // feeds is an array of temperature datas as can be seen if put api url on browser
        const feeds = data.feeds;

        // map through temperature values aka through the array
        const temperatures = feeds.map((feed) => ({
            time: feed.created_at,
            temp: parseFloat(feed.field1),
        }));
        document.getElementById('output').textContent = JSON.stringify(temperatures);
    })
    .catch((error) => {
        console.error('Error fetching data', error);
        document.getElementById('output').textContent = 'Error loading data';
    });
