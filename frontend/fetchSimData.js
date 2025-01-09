const server_url = 'http://localhost:21110/';

let actorTile;
let actorElement;
let mapGridArray;
let tick = 1;

document.addEventListener('DOMContentLoaded', setup)

//Initial Setup Functions
function setup(){
  console.log("Initiating Setup...")
  fetchLevel(setupMap)
  setInterval(fetchAllData, 1000);
  console.log("Setup Complete!")
}

function setupMap(data){
  const mapHeight = data.level.height;
  const mapWidth = data.level.width;

  const levelContainer = document.getElementById("level-container");
  setLevelContainerSize(levelContainer ,mapWidth, mapHeight);
  setupTileElements(levelContainer, data.level.grid);

  setupActor(data.actor[0], data.level.width);
}

function setLevelContainerSize(levelContainer ,mapWidth, mapHeight){
  //Adjust according to level-tile css class
  const tileSize = 20;
  const tileMargin = 1;

  levelContainer.style.width = mapWidth * (tileSize + tileMargin*4) + 'pt';
  levelContainer.style.height = mapHeight * (tileSize + tileMargin*4) + 'pt';
}

function setupTileElements(levelContainer, grid){
  grid.forEach((tile, i) => {
    const newTile = document.createElement("div");
    newTile.classList.add("level-tile");
    newTile.id = "tile-"+i;
    levelContainer.appendChild(newTile);
  })
}

function setupActor(actor, mapWidth){
  actorElement = document.createElement("div");
  actorElement.classList.add("actor");

  updateActor(actor, mapWidth);
}

//Periodic Update Functions
function fetchAllData() {
  console.log("Updating World...");
  fetchLevel(updateWorld)
}

function updateWorld(newData){
  console.log("Received World Data:\n", newData);

  updateTiles(newData.level.grid);

  const actor = newData.actor[0];
  removeActorFromTile();
  updateActor(actor, newData.level.width);

  console.log(`Updated Tick ${tick++}!`);
}

function updateTiles(grid){
  for(let i=0; i<grid.length; i++){
    const tileElement = document.getElementById("tile-"+i);
    tileElement.innerHTML = '';

    checkTileFlags(grid[i], tileElement);
  }
}

function checkTileFlags(tile, tileElement){
  const tf_Underwater = 1 << 0;
  const tf_Protein    = 1 << 1;
  const tf_Sugar      = 1 << 2;
  const tf_Vitamin    = 1 << 3;
  const tf_Fat        = 1 << 4;
  const tf_Collidable = 1 << 5;
  const tf_OtherActor = 1 << 6;
  const tf_Hidden     = 1 << 7;

  if(tile & tf_Hidden){ //Hidden

  }
  if(tile & tf_OtherActor){ //Other Actor

  }
  if(tile & tf_Collidable){ //Collidable
    tileElement.background = '#e40f0f'
  }
  if(tile & tf_Fat){ //Fat
    let newItem = document.createElement("div");
    newItem.classList.add('food');
    newItem.innerText = 'F'
    tileElement.appendChild(newItem);
  }
  if(tile & tf_Vitamin){ //Vitamin
    let newItem = document.createElement("div");
    newItem.classList.add('food');
    newItem.innerText = 'V'
    tileElement.appendChild(newItem);
  }
  if(tile & tf_Sugar){ //Sugar
    let newItem = document.createElement("div");
    newItem.classList.add('food');
    newItem.innerText = 'S'
    tileElement.appendChild(newItem);
  }
  if(tile & tf_Protein){ //Protein
    let newItem = document.createElement("div");
    newItem.classList.add('food');
    newItem.innerText = 'P'
    tileElement.appendChild(newItem);
  }
  if(tile & tf_Underwater){ //Underwater
    tileElement.style.backgroundColor = '#0000ff';
  }
}

function removeActorFromTile(){
  //Todo: Adjust for other information
  actorTile.innerHTML = '';
}

function updateActor(actor, mapWidth){
  actorTile = document.getElementById("tile-" + (actor.posX + mapWidth * actor.posY));
  actorTile.appendChild(actorElement);
}

//Net functions
function handleError(error){
  console.error("Encountered error during fetch:\n", error);
}

function load_url(url, callback, payload, failure_callback) {
  var xmlhttp;

  console.log("Sending request to '" + url + "':");
  //console.log(payload);

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

function fetchLevel(callback){
  load_backend_url('getLevel', callback,
    {}, handleError);
}
