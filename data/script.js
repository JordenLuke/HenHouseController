function fetchDataAndUpdate() {
    // Make AJAX request to fetch JSON data
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (xhr.readyState == XMLHttpRequest.DONE) {
            if (xhr.status == 200) {
                var data = JSON.parse(xhr.responseText);
                // Update HTML elements with fetched data
                document.getElementById("indoortemp").innerText = data.indoorTemperature;
                document.getElementById("indoorhigh").innerText = data.indoorHigh + "°F";
                document.getElementById("indoorlow").innerText = data.indoorLow + "°F";
                document.getElementById("indoorhum").innerText = data.indoorHumidity;

                document.getElementById("outdoortemp").innerText = data.outdoorTemperature;
                document.getElementById("outdoorhigh").innerText = data.outdoorHigh + "°F";
                document.getElementById("outdoorlow").innerText = data.outdoorLow + "°F";
                document.getElementById("outdoorhum").innerText = data.outdoorHumidity;

                document.getElementById("watertemp").innerText = data.waterTemperature;
                document.getElementById("waterhigh").innerText = data.waterHigh + "°F";
                document.getElementById("waterlow").innerText = data.waterLow + "°F";
            } else {
                console.error('Failed to fetch data');
            }
        }
    };
    xhr.open("GET", "/json", true);
    xhr.send();
}

// Call fetchDataAndUpdate function on page load
window.onload = fetchDataAndUpdate;

// Call fetchDataAndUpdate function every 10 seconds
setInterval(fetchDataAndUpdate, 10000); // 10000 milliseconds = 10 seconds
