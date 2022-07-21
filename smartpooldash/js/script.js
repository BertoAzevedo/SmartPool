var daychart = document.getElementById("daychart").getContext("2d");

let currentDay = ''
let dayGraphData = []
let isErrorShowing = false

function loadTemps(data) {
    let values = Object.values(data[0]);
    document.getElementById("text-temps-panel").innerHTML = values[3] + "°";
    document.getElementById("text-temps-pool").innerHTML = values[2] + "°";
    document.getElementById("text-timestamp").innerHTML = "Ultima atualização: " + values[1];
}

function showTemperatures() {
    document.getElementById("temperature-div").style.display = "flex"
    document.getElementById("text-timestamp").style.display = "block"
    document.getElementById("text-title-pool").style.display = "block"
    document.getElementById("text-title-panel").style.display = "block"
    document.getElementById("text-temps-panel").style.display = "block"
    document.getElementById("text-temps-pool").style.display = "block"
}

function loadDayGraphData(data_temp) {
    let timestamp = []
    let pool_temp = []
    let panel_temp = []

    data_temp.forEach(e => {
        timestamp.push(e.timestamp)
        pool_temp.push(e.pooltemp)
        panel_temp.push(e.paneltemp)
    });

    let panel_data = []

    timestamp.forEach((e, i) => {
        if (parseFloat(panel_temp[i]) > 0) {
            let time = moment(e)
            panel_data.push({
                t: time,
                y: panel_temp[i]
            })
        }
    })

    let pool_data = []

    timestamp.forEach((e, i) => {
        if (parseFloat(panel_temp[i]) > 0) {
            let time = moment(e)
            pool_data.push({
                t: time,
                y: pool_temp[i]
            })
        }
    })

    let dataset = [{
            label: 'Painel',
            data: panel_data,
            fill: false,
            borderColor: [
                'rgba(255,99,132,1)',
            ],
            borderWidth: 1
        },
        {
            label: 'Piscina',
            data: pool_data,
            fill: false,
            borderColor: [
                'rgba(70, 164, 246, 1)',
            ],
            borderWidth: 1
        }
    ]

    var myChart = new Chart(daychart, {
        type: 'line',
        data: {
            labels: timestamp,
            datasets: dataset
        },
        options: {
            scales: {
                xAxes: [{
                    type: 'time',
                    time: {
                        unit: 'hour'
                    }
                }]
            },
            elements: {
                point: {
                    radius: 0
                }
            }
        }
    });

    showTemperatures()
    showDayTempChart()
    hideLoadingIndicator()
}

function hideLoadingIndicator() {
    document.getElementById("spinner").style.display = "none"
}

function showDayTempChart() {
    document.getElementById("daychart").style.display = "block"
}

// Create the XHR object.
function createCORSRequest(method, url) {
    var xhr = new XMLHttpRequest()
    if ("withCredentials" in xhr) {
        // XHR for Chrome/Firefox/Opera/Safari.
        xhr.open(method, url, true)
    } else if (typeof XDomainRequest != "undefined") {
        // XDomainRequest for IE.
        xhr = new XDomainRequest()
        xhr.open(method, url)
    } else {data
        xhr = null
    }
    return xhr
}

// Make the actual CORS request.
function makeCorsRequest(url, flag) {
    var xhr = createCORSRequest('GET', url)
    if (!xhr) {
        alert('CORS not supported')
        return
    }

    // Response handlers.
    xhr.onload = function () {
        var text = xhr.responseText
        var obj = JSON.parse(text);
        if (Array.isArray(obj)) {
            if (flag == 'last') {
                loadTemps(obj)
            } else if (flag == 'day') {
                loadDayGraphData(obj)
            }
        } else {
            showError()
        }
    }

    xhr.onerror = function () {
        showError()
    }

    xhr.send()
}

function showError() {
    console.log('Woops, there was an error making the request.')
    if (!isErrorShowing) {
        var img = document.createElement("img");
        img.src = "img/calor.jpg";
        var src = document.getElementById("error-image");
        src.appendChild(img);
        src.style.display = 'flex'

        var error = document.createElement("h5")
        error.innerHTML = "Woops, algo de errado não está certo ¯\\_(ツ)_/¯"
        var src = document.getElementById("error-text");
        src.appendChild(error);
        src.style.display = 'block'

        isErrorShowing = true
        hideLoadingIndicator()
    }
}

function getLastTemps() {
    let lastTempUrl = 'http://yourip:8089/last'
    let flag = 'last'
    makeCorsRequest('https://api.allorigins.win/raw?url=' + lastTempUrl, flag)
}

function getCurrentDayTemps() {
    var today = new Date();
    currentDay = today.getFullYear() + '-' + String(today.getMonth() + 1).padStart(2, '0') + '-' + String(today.getDate()).padStart(2, '0');
    let currentDayTempsUrl = 'http://yourip:8089/day/' + currentDay
    let flag = 'day'
    makeCorsRequest('https://api.allorigins.win/raw?url=' + currentDayTempsUrl, flag)
}

function init() {
    getLastTemps()
    getCurrentDayTemps()
}

init()