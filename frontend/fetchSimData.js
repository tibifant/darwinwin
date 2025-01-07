const server_url = 'http://localhost:21110/';

let actorTile;
let actor;
let mapWidth;
let mapHeight;
let mapGridArray;

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
  mapHeight = data.level.height;
  mapWidth = data.level.width;

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
  console.log("Received World Data:\n", newData);

  mapGridArray = convertGridTo2dArray(newData.level.grid)
  updateTiles(mapGridArray);

  const actor = newData.actor[0];
  updateActor(actor.posX, actor.posY, actor.lookDir);
}

function convertGridTo2dArray(grid) {
  let gridArray = [];

  for(let i=0; i<mapWidth; i++){
    gridArray.push([])
    for (let j = 0; j < mapHeight; j++){
      gridArray[i][j] = grid[i * mapWidth + j ];
    }
  }

  return gridArray;
}

function updateTiles(tileArray){
  for(let i=0; i<tileArray.length; i++){
    for (let j = 0; j < tileArray[i].length; j++){
      const tileElement = document.getElementById("tile-"+i+"-"+j);
      tileElement.innerHTML = '';

      const tileInfoBinary = tileArray[i][j].toString(2).padStart(8, "0");

      if(tileInfoBinary[0] === '1'){ //Hidden

      }
      if(tileInfoBinary[1] === '1'){ //Other Actor

      }
      if(tileInfoBinary[2] === '1'){ //Collidable
        tileElement.background = '#e40f0f'
      }
      if(tileInfoBinary[3] === '1'){ //Fat
        let newItem = document.createElement("div");
        newItem.classList.add('food');
        newItem.innerHTML = 'F'
        tileElement.appendChild(newItem);
      }
      if(tileInfoBinary[4] === '1'){ //Vitamin
        let newItem = document.createElement("div");
        newItem.classList.add('food');
        newItem.innerHTML = 'V'
        tileElement.appendChild(newItem);
      }
      if(tileInfoBinary[5] === '1'){ //Sugar
        let newItem = document.createElement("div");
        newItem.classList.add('food');
        newItem.innerHTML = 'S'
        tileElement.appendChild(newItem);
      }
      if(tileInfoBinary[6] === '1'){ //Protein
        let newItem = document.createElement("div");
        newItem.classList.add('food');
        newItem.innerHTML = 'P'
        tileElement.appendChild(newItem);
      }
      if(tileInfoBinary[7] === '1'){ //Underwater
        tileElement.style.backgroundColor = '#0000ff';
      }
    }
  }
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
