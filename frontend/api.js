const server_url = 'http://localhost:21110/';

let actorElement;
let mapGridArray;
let tick = 1;
let actorStats;
let updateInfo = false;

const tileFlags = new Map([
  ['Underwater', 1 << 0],
  ['Protein', 1 << 1],
  ['Sugar', 1 << 2],
  ['Vitamin', 1 << 3],
  ['Fat', 1 << 4],
  ['Collidable', 1 << 5],
  ['OtherActor', 1 << 6],
  ['Hidden', 1 << 7],
])

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
    newTile.addEventListener("click", infoClick)
    levelContainer.appendChild(newTile);
  })
}

function setupActor(actor, mapWidth){
  actorElement = document.createElement("div");
  actorElement.classList.add("actor");
  actorElement.id = "actor";
  actorElement.addEventListener("click", infoClick)

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
  updateActor(actor, newData.level.width);
  if(updateInfo){
    showStatsOfElement(updateInfo);
    updateInfo = false;
  }
  console.log(`Updated Tick ${tick++}!`);
}

function updateTiles(grid){
  for(let i=0; i<grid.length; i++){
    const tileElement = document.getElementById("tile-"+i);
    tileElement.innerHTML = '';
    tileElement.style.backgroundColor = '#567345';

    mapGridArray = grid;
    checkTileFlags(grid[i], tileElement);
  }
}

function checkTileFlags(tile, tileElement){
  if(hasTileCondition(tile, "Hidden")){

  }
  if(hasTileCondition(tile, "OtherActor")){

  }
  if(hasTileCondition(tile, "Collidable")){
    tileElement.style.backgroundColor = '#3e3a3a'
    return;
  }
  if(hasTileCondition(tile, "Fat")){
    tileElement.appendChild(createFoodOf('F'));
  }
  if(hasTileCondition(tile, "Vitamin")){
    tileElement.appendChild(createFoodOf('V'));
  }
  if(hasTileCondition(tile, "Sugar")){
    tileElement.appendChild(createFoodOf('S'));
  }
  if(hasTileCondition(tile, "Protein")){
    tileElement.appendChild(createFoodOf('P'));
  }
  if(hasTileCondition(tile, "Underwater")){ //Underwater
    tileElement.style.backgroundColor = '#0000ff';
  }
}

function hasTileCondition(tile, condition){
  return !!(tile & tileFlags.get(condition));
}

function createFoodOf(type){
  const newItem = document.createElement("div");
  newItem.classList.add('food');
  newItem.innerText = type;
  return newItem;
}

function updateActor(actor, mapWidth){
  const actorTile = document.getElementById("tile-" + (actor.posX + mapWidth * actor.posY));
  actorTile.appendChild(actorElement);
  actorStats = actor;
}

//Stats view functions
function infoClick(event){
  showStatsOfElement(event.target.id);
}

function showStatsOfElement(targetElementId){
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  const idBody = targetElementId.split('-')[0];
  switch (idBody) {
    case 'actor':
      showActorStats(targetElementId, infoLabelsElement, infoValuesElement, optionsElement);
      break;
    case 'tile':
      showTileStats(targetElementId, infoLabelsElement, infoValuesElement, optionsElement);
      break;
    default:
      console.error("Error: targetElementId not recognized");
  }
}

//Todo: Clean, optimize
function showActorStats(id, infoLabelsElement, infoValuesElement, optionsElement){ //id for when multiple actors exist
  const actorActions = [
    "move",
    "move 2",
    "turn left",
    "turn right",
    "eat"
  ]
  const lookDirections = [
    "left",
    "up",
    "right",
    "down",
  ]

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

//Todo: Clean, optimize
function showTileStats(id, infoLabelsElement, infoValuesElement, optionsElement){
  const tileIndex = id.split('-')[1];
  const x = tileIndex % 32;
  const y = Math.floor(tileIndex / 32);

  const labels = document.createElement('label')
  labels.innerText = "Tile ID:\nX:\nY:\n\n" +
    "Underwater:\nProtein:\nSugar:\nVitamin\nFat\n" +
    "Collidable:\nOther Actor:\nHidden:";
  infoLabelsElement.appendChild(labels);
  const values = document.createElement('label');
  values.innerText = `${id}\n${x}\n${y}\n
  ${hasTileCondition(mapGridArray[tileIndex],"Underwater")}\n${hasTileCondition(mapGridArray[tileIndex], "Protein")}
  ${hasTileCondition(mapGridArray[tileIndex], "Sugar")}\n${hasTileCondition(mapGridArray[tileIndex], "Vitamin")}
  ${hasTileCondition(mapGridArray[tileIndex], "Fat")}\n${hasTileCondition(mapGridArray[tileIndex], "Collidable")}
  ${hasTileCondition(mapGridArray[tileIndex], "OtherActor")}\n${hasTileCondition(mapGridArray[tileIndex], "Hidden")}`;
  infoValuesElement.appendChild(values);

  const optionsButtonsElement = document.createElement('div');
  optionsButtonsElement.classList.add('stats-subsection-column');

  tileFlags.forEach((value, key) => {
    const button = document.createElement('button');
    button.id = "option-button-tile-"+tileIndex+"-"+key;
    button.innerText = "Toggle "+key;
    button.addEventListener('click', initiateSetTileRequest);
    optionsButtonsElement.appendChild(button);
  })
  optionsElement.appendChild(optionsButtonsElement);
}

//Set functions
function initiateActorActionRequest(event){
  const actionId = event.target.id.slice(-1);
  postActorAction(actionId);
  updateInfo = "actor";
}

function initiateSetTileRequest(event){
  const tileIndex = event.target.id.split('-')[3];
  const tile = mapGridArray[tileIndex];
  const condition = event.target.id.split('-')[4];
  const x = tileIndex % 32;
  const y = Math.floor(tileIndex / 32);

  let newTile;
  if(hasTileCondition(tile, condition)){
    newTile = tile - tileFlags.get(condition);
  }
  else {
    newTile = tile + tileFlags.get(condition);
  }

  const payload = {
    x: x,
    y: y,
    value: newTile
  }

  postTileSetRequest(payload);
  updateInfo = "tile-"+tileIndex;
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

function postTileSetRequest(payload){
  load_backend_url('setTile', {},
    payload, handleError);
}
