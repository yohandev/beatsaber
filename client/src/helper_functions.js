function httpGet() {
    let xmlHttpReq = new XMLHttpRequest();
    xmlHttpReq.open("GET", "https://608dev-2.net/sandbox/sc/team27/web_server.py", false); 
    xmlHttpReq.send(null);
    return xmlHttpReq.responseText;
}
