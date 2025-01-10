const server_url = 'http://localhost:21110/';

let actorTile;
let actorElement;
let mapGridArray;
let tick = 1;
let actorStats;

const lookDirections = [
  "left",
  "up",
  "right",
  "down",
]

const actorActions = [
  "move",
  "move 2",
  "turn left",
  "turn right",
  "eat"
]

document.addEventListener('DOMContentLoaded', setup);

//Initial Setup Functions
function setup(){
  console.log("Initiating Setup...");
  fetchLevel(setupMap);
  setupStatsWindow();
  setInterval(fetchAllData, 1000);
  console.log("Setup Complete!");
}

function setupStatsWindow(){
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const optionsElement = document.getElementById("stats-options");

  let statsMessage = document.createElement("label")
  statsMessage.innerText = "Click on tile or actor to reveal stats..."
  infoLabelsElement.appendChild(statsMessage);
  statsMessage = document.createElement("label")
  statsMessage.innerText = "Click on tile or actor to see options..."
  optionsElement.appendChild(statsMessage);
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
  actorElement.id = "actor";
  actorElement.addEventListener("click", showStatsOfElement)

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
  actorStats = actor;
}

//Stats view functions
function showStatsOfElement(event){
  const targetElement = event.target;

  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  switch (targetElement.id){
    case 'actor':
      showActorStats(targetElement.id, infoLabelsElement, infoValuesElement, optionsElement);
      break;
    default:
      console.error("Error: targetElementId not recognized");
  }
}

function showActorStats(id, infoLabelsElement, infoValuesElement, optionsElement){ //id for when multiple actors exist

  const labels = document.createElement('label')
  labels.innerText = "Actor ID:\nPosX:\nPosY:\nLookDir:\n\n" +
    "Energy:\nAir:\nProtein\nSugar:\nVitamin:\nFat:";
  infoLabelsElement.appendChild(labels);
  const values = document.createElement('label');
  values.innerText = `${id}\n${actorStats.posX}\n${actorStats.posY}\n${lookDirections[actorStats.lookDir]}\n
  ${actorStats.stats[0]}\n${actorStats.stats[1]}\n${actorStats.stats[2]}
  ${actorStats.stats[3]}\n${actorStats.stats[4]}\n${actorStats.stats[5]}\n`;
  infoValuesElement.appendChild(values);

  const optionsButtonsElement = document.createElement('div');
  optionsButtonsElement.classList.add('stats-subsection-column');
  actorActions.forEach((action, i) => {
    const button = document.createElement('button');
    button.id = "option-button-"+i;
    button.innerText = action;
    button.addEventListener('click', initiateActorActionRequest)
    optionsButtonsElement.appendChild(button);
  })
  optionsElement.appendChild(optionsButtonsElement);
}

//Set functions
function initiateActorActionRequest(event){
  const actionId = event.target.id.slice(-1);
  postActorAction(actionId);
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
          failure_callback(e);
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

function postActorAction(id){
  load_backend_url('manualAct', {},
    {actionId: id}, handleError);
}
