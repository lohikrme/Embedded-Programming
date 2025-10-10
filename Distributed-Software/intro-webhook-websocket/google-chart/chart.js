google.charts.load('current', { packages: ['corechart'] });

// make anonymous async function, so we can wait until data fetch is ok
google.charts.setOnLoadCallback(async () => {
    let dataArray = await fetchData();
    drawChart(dataArray);
});

// read the local csv file to be the data
async function fetchData() {
    const response = await fetch('feeds.csv');
    const text = await response.text();
    // prints pure csv text
    //console.log(text);
    // split csv to suitable rows
    const rows = text.trim().split('\n');
    // split rows by suitable separator
    let data = rows.map((row) => row.split(','));
    // prints array of csv data containing all headers
    //console.log(data);
    // remove unneeded headers from end of rows
    data = data.map((row) => row.slice(0, 4));
    // prints data with first 4 columns
    //console.log(data);
    // remove unneeded id header from index 1
    // IMPORTANT: notice that here concat combines two rows into one row!!!
    data = data.map((row) => row.slice(0, 1).concat(row.slice(2)));
    // print data without id header
    //console.log(data);
    // filter away rows where field1 or field2 are missing data
    data = data.filter((row) => {
        const f1 = row[1];
        const f2 = row[2];
        return f1 !== '0' && f1 !== '' && f2 !== '0' && f2 !== '';
    });
    // print data with only rows of intact field1 and field2
    //console.log(data);
    // rename headers to have big letters and short names
    data[0][0] = 'Time';
    data[0][1] = 'Temperature';
    data[0][2] = 'Humidity';
    // print data with new headers
    console.log(data);
    // make datas to be numeric rather than strings
    // IMPORTANT: notice that here concat combines matrix [[]] to a []
    // so it will be a new row inside the matrix
    // also trim last useless letters from time
    data = [data[0]].concat(
        data.slice(1).map((row) => {
            return [row[0].slice(11, -6), Number(row[1]), Number(row[2])];
        })
    );

    // print datas to make sure data is numeric
    console.log(data);
    return data;
}

function drawChart(dataArray) {
    let data = google.visualization.arrayToDataTable(dataArray);

    let options = {
        title: 'DHT22 data',
        hAxis: { title: 'Time', titleTextStyle: { color: '#333' } },
        vAxis: { minValue: 0 },
    };

    let chart = new google.visualization.LineChart(document.getElementById('chart_div'));
    chart.draw(data, options);
}
