const { Client } = require("pg")
const express = require("express")
const moment = require("moment")
const fs = require('fs')
const app = express();

const client = new Client({
    "user": "postgres",
    "password": "12345",
    "host": "localhost",
    "port": 5432,
    "database": "homeautomation"
})

app.get("/data", async (req, res) => {
    const rows = await getData()
    res.setHeader("content-type", "application/json")
    res.send(JSON.stringify(rows))
})

app.get("/last", async (req, res) => {
    const rows = await getLastTemps()
    res.setHeader("content-type", "application/json")
    res.send(JSON.stringify(rows))
})

app.get("/day/:day", async (req, res) => {
    const rows = await getDayTemps(req.params.day)
    res.setHeader("content-type", "application/json")
    res.send(JSON.stringify(rows))
})

app.listen(8089, () => {
    console.log("Smartpool API listening on port 8089!")
})

setTimeout(function () { connect() }, 10000);

async function connect() {
    try {
        await client.connect();
        console.log(`Connected to database..`)
    }
    catch (e) {
        console.log(`Failed to connect to database ${e}`)
    }
}

async function getData() {
    try {
        const results = await client.query("select poolid, timestamp, pooltemp, paneltemp from pooltemps;");
        console.log("Requested all data...")
        return results.rows;
    }
    catch (e) {
        return [];
    }
}

async function getLastTemps() {
    try {
        const results = await client.query("SELECT poolid, timestamp, pooltemp, paneltemp FROM pooltemps ORDER BY TIMESTAMP DESC LIMIT 1;");
        console.log("Requested last temperatures data...")
        return results.rows;
    }
    catch (e) {
        return [];
    }
}

async function getDayTemps(day) {
    try {
        console.log('Requested data from day: ', day + "...")
        if (moment(day, "YYYY-MM-DD", true).isValid()) {
            let q = "SELECT poolid, timestamp, pooltemp, paneltemp FROM pooltemps WHERE timestamp > '" + day + " 00:00' AND timestamp < '" + day + " 23:59' ORDER BY timestamp;"
            const results = await client.query(q);
            return results.rows;
        } else {
            return []
        }
    }
    catch (e) {
        return [];
    }
}
