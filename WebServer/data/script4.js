var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

let currentPage = 1;
const itemsPerPage = 10;
let historyData = [];

window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    fetchHistoryData();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established
function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log(event.data);
    // Check if the message is a relay history message
    if (event.data.startsWith("HISTORY|")) {
        let jsonString = event.data.substring(8); // Remove 'HISTORY|' prefix
        try {
            let entry = JSON.parse(jsonString);
            addNewEntry(entry);
        } catch (e) {
            console.error('Error parsing JSON:', e);
        }
    }
}

function fetchHistoryData() {
    fetch('/getHistory')
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json()
    })
    .then(data => {
        console.log('Fetched data:', data);
        // data is an array of feed data entries
        historyData = data.map(item => {
            try {
                return JSON.parse(item.value);
            } catch (e) {
                console.error('Error parsing entry:', e);
                return null;
            }
        }).filter(item => item !== null); // Filter out any null entries due to parsing errors

        // Sort historyData by timestamp descending
        historyData.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));

        displayPage(1);
        setupPagination();
    })
    .catch(error => console.error('Error fetching history data:', error));
}

function displayPage(pageNumber) {
    currentPage = pageNumber;
    const tbody = document.querySelector('#historyTable tbody');
    tbody.innerHTML = '';

    const startIndex = (pageNumber - 1) * itemsPerPage;
    const endIndex = Math.min(startIndex + itemsPerPage, historyData.length);

    for (let i = startIndex; i < endIndex; i++) {
        const entry = historyData[i];
        const tr = document.createElement('tr');

        const relayNameTd = document.createElement('td');
        relayNameTd.textContent = `Relay ${entry.relayId}`;
        tr.appendChild(relayNameTd);

        const stateTd = document.createElement('td');
        stateTd.textContent = entry.state;
        tr.appendChild(stateTd);

        const timestampTd = document.createElement('td');
        timestampTd.textContent = formatDateTime(entry.timestamp);
        tr.appendChild(timestampTd);

        tbody.appendChild(tr);
    }
}

function setupPagination() {
    const paginationDiv = document.getElementById('pagination');
    paginationDiv.innerHTML = '';

    const totalPages = Math.ceil(historyData.length / itemsPerPage);

    for (let i = 1; i <= totalPages; i++) {
        const btn = document.createElement('button');
        btn.textContent = i;
        if (i === currentPage) {
            btn.classList.add('active');
        }
        btn.addEventListener('click', () => {
            displayPage(i);
            updateActiveButton(i);
        });
        paginationDiv.appendChild(btn);
    }
}

function updateActiveButton(pageNumber) {
    const buttons = document.querySelectorAll('#pagination button');
    buttons.forEach(btn => {
        if (parseInt(btn.textContent) === pageNumber) {
            btn.classList.add('active');
        } else {
            btn.classList.remove('active');
        }
    });
}

function addNewEntry(entry) {
    // Add to the beginning of historyData
    historyData.unshift(entry);

    // If the current page is the first page, update display
    if (currentPage === 1) {
        displayPage(1);
    }

    setupPagination();
}

function formatDateTime(dateTimeStr) {
    const date = new Date(dateTimeStr);
    if (isNaN(date)) return dateTimeStr; // Return as is if invalid date

    const options = {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    };

    return date.toLocaleString('en-GB', options);
}
