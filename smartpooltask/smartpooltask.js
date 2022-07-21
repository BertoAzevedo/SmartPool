const { Client } = require("pg")
var getJSON = require("get-json")
const cron = require("node-cron");

const client = new Client({
    "user": "postgres",
    "password": "12345",
    "host": "localhost",
    "port": 5432,
    "database": "homeautomation"
})

cron.schedule("* * * * *", () => {
    let values = []
    var url = 'http://192.168.1.94/data';

    getJSON(url)
        .then(function (response) {
            values = Object.values(response);
            console.log("Panel temp: ", values[0])
            console.log("Pool temp: ", values[1])
            insertNewTemp(Math.round(values[0] * 10) / 10, Math.round(values[1] * 10) / 10)
        }).catch(function (error) {
            console.log(error.code);
        });
})

async function insertNewTemp(panel_temp, pool_temp) {
    var today = new Date();
    var dd = String(today.getDate()).padStart(2, '0');
    var mm = String(today.getMonth() + 1).padStart(2, '0'); //January is 0!
    var yyyy = today.getFullYear();
    var hh = (today.getHours()<10?'0':'') + today.getHours()
    var MM = (today.getMinutes()<10?'0':'') + today.getMinutes()
    today = yyyy + '-' + mm + '-' + dd + " " + hh + ":" + MM;

    try {
        await client.query("insert into pooltemps (poolid, timestamp, pooltemp, paneltemp) values ($1, $2, $3, $4)", [1, today, pool_temp, panel_temp]);
        console.log("Insert completed at " + today + ".")
        return true
    }
    catch (e) {
        console.log("Failed to insert... Error: " + e)
        return false;
    }
}

async function connect() {
    try {
        await client.connect();
        console.log(`Connected to database.`)
    }
    catch (e) {
        console.log(`Failed to connect to database ${e}`)
    }
}

setTimeout(function(){ connect() }, 10000);
