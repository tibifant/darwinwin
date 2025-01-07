const server_url = 'http://localhost:21110/';

let actorTile;
let actor;

setup();

function setup(){
  setUpActor();
  fetchMap();
  setInterval(fetchAllData, 1000);
}
//Initial Setup Functions
function fetchMap(){
  load_backend_url('getLevel', (response) => {setUpMap(response)},
    {}, (e) => {handleError(e)});
}

function setUpActor(){
  actor = document.createElement("div");
  actor.classList.add("actor");
}

function setUpMap(data){
  const mapHeight = data.level.height;
  const mapWidth = data.level.width;

  setUpMapColumns(mapWidth);

  for(let i=0; i<mapWidth; i++){
    const column = document.getElementById("level-column-" + i)

    for (let j = 0; j < mapHeight; j++){
      let newTile = document.createElement("div");
      newTile.id = 'tile-' + i + '-' + j;
      newTile.classList.add('level-tile');
      column.appendChild(newTile);
    }
  }

  actorTile = document.getElementById("tile-"+data.actor[0].posX+"-"+data.actor[0].posY);
}

function setUpMapColumns(mapWidth) {
  const levelContainer = document.getElementById("level-container");

  for (let i = 0; i < mapWidth; i++) {
    let newColumn = document.createElement("div");
    newColumn.id = 'level-column-' + i;
    newColumn.classList.add("level-column");
    levelContainer.appendChild(newColumn);
  }
}

//Periodic Update Functions
function fetchAllData() {
  fetchLevelData();
}

function fetchLevelData(){
  load_backend_url('getLevel', (response) => {updateLevelData(response)},
    {}, (e) => {handleError(e)});
}

function updateLevelData(newData){
  console.log("Received World Data:", newData)

  const actor = newData.actor[0];

  updateActor(actor.posX, actor.posY, actor.lookDir);
}

function updateActor(posX, posY, lookDirection){
  removeActorFromTile()

  actorTile = document.getElementById("tile-"+posX+"-"+posY);

  actorTile.appendChild(actor);
}

function removeActorFromTile(){
  //Adjust for other information
  actorTile.innerHTML = '';
}

//Net functions
function handleError(error){
  console.error("Encountered error during fetch:\n", error);
}

function load_url(url, callback, payload, failure_callback) {
  var xmlhttp;

  console.log("Sending request to '" + url + "':");
  console.log(payload);

  if (window.XMLHttpRequest)
    xmlhttp = new XMLHttpRequest();
  else
    xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");

  xmlhttp.onreadystatechange = function () {
    if (xmlhttp.readyState == 4) {
      if (xmlhttp.status >= 200 && xmlhttp.status < 300) {
        try {
          let obj = JSON.parse(xmlhttp.responseText);
          callback(obj);
        } catch (e) {
          failure_callback(false);
        }
      } else { // TODO: handle all relevant error codes
        if (xmlhttp.status == 403) {

        } else {
          failure_callback(false);
        }
      }
    }
  }

  xmlhttp.ontimeout = (e) => {
    failure_callback();
  };

  xmlhttp.timeout = 7500;
  xmlhttp.open("POST", url, true);
  xmlhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
  xmlhttp.send(JSON.stringify(payload));
}

function load_backend_url(url, callback, payload, failure_callback) {
  load_url(server_url + url, callback, payload, failure_callback);
}
