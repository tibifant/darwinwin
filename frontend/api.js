const server_url = 'http://localhost:21110/';

let actorElement;
let mapGridArray;
let tick = 1;
let actorStats;
let updateInfo = false;
let mapWidth;

const tileFlags = {
      Underwater: 1 << 0,
      Protein: 1 << 1,
      Sugar: 1 << 2,
      Vitamin: 1 << 3,
      Fat: 1 << 4,
      Collidable: 1 << 5,
      OtherActor: 1 << 6,
      Hidden: 1 << 7,
    }

document.addEventListener('DOMContentLoaded', setup);

//Initial Setup Functions
function setup(){
  console.log("Initiating Setup...");
  fetchLevel(setupMap);
  setupStatsWindow();
  setupViewCone();
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
  mapWidth = data.level.width;

  const levelContainer = document.getElementById("level-container");
  setLevelContainerSize(levelContainer, mapHeight);
  setupTileElements(levelContainer, data.level.grid);

  setupActor(data.actor[0]);
}

function setLevelContainerSize(levelContainer, mapHeight){
  //Adjust according to level-tile css class
  const tileSize = 20;
  const tileMargin = 1;
  const containerWidth = mapHeight * (tileSize + tileMargin*4) + 'pt';
  const containerHeight = mapWidth * (tileSize + tileMargin*4) + 'pt';

  levelContainer.style.width = containerWidth;
  levelContainer.style.height = containerHeight;

  levelContainer.style.minWidth = containerWidth;
  levelContainer.style.minHeight = containerHeight;
}

function setupTileElements(levelContainer, grid){
  grid.forEach((tile, i) => {
    const newTile = document.createElement("div");
    newTile.classList.add("level-tile");
    newTile.id = "tile-"+i;
    newTile.addEventListener("click", event => {showTileStats(event.target.id)})
    levelContainer.appendChild(newTile);
  })
}

function setupActor(actor){
  actorElement = document.createElement("div");
  actorElement.classList.add("actor");
  actorElement.id = "actor";
  actorElement.addEventListener("click", showActorStats)

  updateActor(actor);
}

function setupViewCone(){
  //TODO: Possible to make this more dynamic?
  const viewConeSize = 8;
  const positions = [
    { row: 1, col: 2 },
    { row: 2, col: 1 },
    { row: 2, col: 2 },
    { row: 2, col: 3 },
    { row: 3, col: 1 },
    { row: 3, col: 2 },
    { row: 3, col: 3 },
    { row: 4, col: 2 },
  ];

  const grid = document.getElementById('view-cone-grid') || {}

  if(grid.length === 0){
    console.error("Error: Could not find view-cone-grid element");
  } else {
    for (let i = 0; i < viewConeSize; i++) {
      const tile = document.createElement('div');
      tile.classList.add('view-cone-tile');
      tile.style.gridRow = positions[i].row;
      tile.style.gridColumn = positions[i].col;
      tile.style.backgroundColor = '#c6c6c6'
      tile.id = 'view-cone-tile-' + i;
      grid.appendChild(tile);
    }
  }
}


//###################################################################
//##################Periodic Update Functions########################
//###################################################################

function fetchAllData() {
  console.log("Updating World...");
  fetchLevel(updateWorld)
}

function updateWorld(newData){
  console.log("Received World Data:\n", newData);

  updateTiles(newData.level.grid);

  const actor = newData.actor[0];
  updateActor(actor);
  if(updateInfo){ //TODO: Temporary solution
    switch (updateInfo.split("-")[0]) {
      case "actor":
        showActorStats(new CustomEvent("", {
          detail: {
            id: updateInfo
          }
        }));
        break;
      case "tile":
        showTileStats(updateInfo);
        break;
    }
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
    if(tileElement.id.split('-')[0] === 'view'){
      tileElement.style.backgroundColor = '';
      tileElement.style.border = '2px solid black';
      return;
    }
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
  return !!(tile & tileFlags[condition]);
}

function createFoodOf(type){
  const newItem = document.createElement("div");
  newItem.classList.add('food');
  newItem.innerText = type;
  return newItem;
}

function updateActor(actor){
  const actorTile = document.getElementById("tile-" + (actor.posX + mapWidth * actor.posY));
  actorTile.appendChild(actorElement);
  actorStats = actor;
  console.log("actorStats updated:");
  console.log(actorStats)
}

//Stats view for Actor functions
function showActorStats(event){
  console.log("Showing Actor Stats");
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  const labelsElement = document.createElement('label')
  const valuesElement = document.createElement('label');

  fillActorLabelsAndValuesElements(labelsElement, valuesElement);

  infoLabelsElement.appendChild(labelsElement);
  infoValuesElement.appendChild(valuesElement);
  optionsElement.appendChild(createActorOptionsButtonsElement());

  showViewCone(event.detail.id);

  event.stopPropagation();
}

function showViewCone(eventElementId){
  //TODO: Change for multiple actors
  //let actor = document.getElementById(eventElementId);
  //let actorStats = actor.actorStats;
  const viewCone = actorStats.viewcone;
  for(let i=0; i<viewCone.length; i++){
    const tile = document.getElementById('view-cone-tile-'+i);
    tile.innerHTML = viewCone[i];
    tile.style.backgroundColor = '#567345';
    checkTileFlags(viewCone[i], tile);
  }
}

function fillActorLabelsAndValuesElements(labels, values){
  const lookDirections = [
    "left",
    "up",
    "right",
    "down",
  ]
  const statDescriptions = [
    "Air",
    "Protein",
    "Sugar",
    "Vitamin",
    "Fat",
    "Energy"
  ]

  for(const key in actorStats ){
    switch (key) {
      case "stats":
        labels.innerHTML += "<br>";
        values.innerHTML += "<br>";
        for (const stat in actorStats.stats ){
          labels.innerHTML += statDescriptions[stat] + "<br>";
          values.innerHTML += actorStats.stats[stat] + "<br>";
        }
        break;
      case "lookDir":
        labels.innerHTML += key + '<br>';
        values.innerHTML += lookDirections[actorStats[key]] + '<br>';
        break;
      default:
        labels.innerHTML += key + '<br>';
        values.innerHTML += actorStats[key] + '<br>';
    }
  }
}

function createActorOptionsButtonsElement(){
  const actorActions = [
    "move",
    "move 2",
    "turn left",
    "turn right",
    "eat"
  ]

  const optionsButtonsElement = document.createElement('div');
  optionsButtonsElement.classList.add('stats-subsection-column');
  actorActions.forEach((action, i) => {
    const button = createActorButton(action, i);
    optionsButtonsElement.appendChild(button);
  })
  return optionsButtonsElement;
}

function createActorButton(action, i){
  const button = document.createElement('button');
  button.id = "option-button-"+i;
  button.innerText = action;
  button.addEventListener('click', initiateActorActionRequest)
  return button;
}

//Stats view for Tile functions
function showTileStats(id){
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  const optionsButtonsElement = document.createElement('div');
  optionsButtonsElement.classList.add('stats-subsection-column');
  const labels = document.createElement('label');
  const values = document.createElement('label');

  fillTileStatsElements(id, labels, values, optionsButtonsElement);

  infoLabelsElement.appendChild(labels);
  infoValuesElement.appendChild(values);
  optionsElement.appendChild(optionsButtonsElement);
}

function fillTileStatsElements(id, labels, values, optionsButtonsElement){
  console.log(1);

  const tileIndex = id.split('-')[1];
  const x = tileIndex % mapWidth;
  const y = Math.floor(tileIndex / mapWidth);

  labels.innerHTML = "Tile ID:<br>X:<br>Y:<br><br>";
  values.innerHTML = `${id}<br>${x}<br>${y}<br><br>`;

  for(const flag in tileFlags){
    labels.innerHTML += flag+"<br>";
    values.innerHTML += hasTileCondition(mapGridArray[tileIndex], flag)+"<br>";

    const button = createTileButton(tileIndex, flag);
    optionsButtonsElement.appendChild(button);
  }
}

function createTileButton(tileIndex, key){
  const button = document.createElement('button');
  button.id = "option-button-tile-"+tileIndex+"-"+key;
  button.innerText = "Toggle "+key;
  button.addEventListener('click', initiateSetTileRequest);
  return button;
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
    newTile = tile - tileFlags[condition];
  }
  else {
    newTile = tile + tileFlags[condition];
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
