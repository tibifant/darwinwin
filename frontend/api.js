const server_url = 'http://localhost:21110/';

//Cached Variables
let actorElements = [];
let worldData;

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

const emptyColor = "rgb(112, 168, 87)";

document.addEventListener('DOMContentLoaded', setup);

// ### Initial Setup Functions ###
function setup(){
  console.log("Initiating Setup...");
  setupControlPanel();
  fetchLevel(setupMap);
  setupStatsWindow();
  setupViewCone();
  console.log("Setup Complete!");
}

function setupControlPanel(){
  fetchTrainingState(switchTrainingButton);
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

function setupMap(){
  const levelContainer = document.getElementById("level-container");
  setLevelContainerSize(levelContainer);
  setupTileElements(levelContainer, worldData.level.grid);

  worldData.actor.forEach((actor, i) => {
    setupActor(actor, i, worldData.level.width);
  })

    updateWorld();
}

function setLevelContainerSize(levelContainer){
  //Adjust according to level-tile css class
  const tileSize = 20;
  const tileMargin = 1;
  const containerWidth = worldData.level.height * (tileSize + tileMargin*4) + 'pt';
  const containerHeight = worldData.level.width * (tileSize + tileMargin*4) + 'pt';

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
    newTile.addEventListener("click", event => {showTileStats(event.target)})
    newTile._index = i;
    newTile._posX = (i % worldData.level.width).toString();
    newTile._posY = Math.floor(i / worldData.level.width).toString();
    levelContainer.appendChild(newTile);
  })
}

function setupActor(actor, index){
  const newActorElement = document.createElement("div");
  newActorElement.classList.add("actor");
  newActorElement.id = "actor-"+index;
  newActorElement._index = index;
  newActorElement.addEventListener("click", event => {
    showActorStats(event.target);
    event.stopPropagation();
  })
  actorElements[index] = newActorElement;

  updateActor(actor, index);
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

// ### Periodic Update Functions ###
function fetchAllData(callback = null) {
  console.log("Updating World...");
  //Tried to avoid lambda but could not find alternative
  fetchLevel(res => {
    updateWorld(callback);
  });
}

function updateWorld(callback){
  console.log("Received World Data:\n", worldData);

  updateTiles();

  worldData.actor.forEach((actor, i) => {
      updateActor(actor, i);
  });
  if (callback) callback();
}

function updateTiles(){
  const grid = worldData.level.grid;
  for(let i=0; i<grid.length; i++){
    const tileElement = document.getElementById("tile-"+i);
    tileElement.innerHTML = '';
    tileElement.style.backgroundColor = emptyColor;
    tileElement._conditionsValue = grid[i];

    checkTileFlags(grid[i], tileElement);
  }
}

function checkTileFlags(tile, tileElement){
  if(hasTileCondition(tile, "Hidden")){
    if(tileElement.id.startsWith('view')){
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
    tileElement.style.backgroundColor = 'rgb(50, 115, 185)';
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

function updateActor(actor, index){
  console.log(worldData)
  const actorTile = document.getElementById("tile-" + (actor.posX + worldData.level.width * actor.posY));
  actorTile.appendChild(actorElements[index]);
  worldData.actor[index] = actor;
}

// ### Stats view for Actor functions ###
function showActorStats(actor){
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  const labelsElement = document.createElement('label')
  const valuesElement = document.createElement('label');

  fillActorLabelsAndValuesElements(actor, labelsElement, valuesElement);

  infoLabelsElement.appendChild(labelsElement);
  infoValuesElement.appendChild(valuesElement);
  optionsElement.appendChild(createActorOptionsButtonsElement(actor));

  showViewCone(actor);
}

function fillActorLabelsAndValuesElements(actor, labels, values){
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

  const actorIndex = actor._index;
  const targetActorStats = worldData.actor[actorIndex];
  labels.innerHTML = "Actor ID:<br>PosX:<br>PosY:<br>LookDir:<br><br>";
  values.innerHTML = `${actor.id}<br>${targetActorStats.posX}<br>${targetActorStats.posY}<br>${lookDirections[targetActorStats.lookDir]}<br><br>`;
  for (const stat in targetActorStats.stats ){
    labels.innerHTML += statDescriptions[stat] + "<br>";
    values.innerHTML += targetActorStats.stats[stat] + "<br>";
  }
}

function showViewCone(actor){
  const viewCone = worldData.actor[actor._index].viewcone;
  for(let i=0; i<viewCone.length; i++){
    const tile = document.getElementById('view-cone-tile-'+i);
    tile.innerText = viewCone[i];
    tile.style.backgroundColor = emptyColor;
    checkTileFlags(viewCone[i], tile);
  }
}

function createActorOptionsButtonsElement(actor){
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
    const button = createActorButton(actor, action, i);
    optionsButtonsElement.appendChild(button);
  })
  return optionsButtonsElement;
}

function createActorButton(actor, action, actionId){
  const button = document.createElement('button');
  button.id = "option-button-"+actionId;
  button._actorIndex = actor._index;
  button._actionId = actionId;
  button.innerText = action;
  button.addEventListener('click', initiateActorActionRequest)
  return button;
}

// ### Stats view for Tile functions ###
function showTileStats(tile){
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

  fillTileStatsElements(tile, labels, values, optionsButtonsElement);

  infoLabelsElement.appendChild(labels);
  infoValuesElement.appendChild(values);
  optionsElement.appendChild(optionsButtonsElement);
}

function fillTileStatsElements(tile, labels, values, optionsButtonsElement){
  labels.innerHTML = "Tile ID:<br>X:<br>Y:<br><br>";
  values.innerHTML = `${tile.id}<br>${tile._posX}<br>${tile._posY}<br><br>`;

  const grid = worldData.level.grid;
  for(const flag in tileFlags){
    labels.innerHTML += flag+"<br>";
    values.innerHTML += hasTileCondition(grid[tile._index], flag)+"<br>";

    const button = createTileButton(tile._index, flag);
    optionsButtonsElement.appendChild(button);
  }
}

function createTileButton(tileIndex, key){
  const button = document.createElement('button');
  button.id = "option-button-tile-"+key;
  button._tileIndex = tileIndex;
  button._conditionKey = key;
  button.innerText = "Toggle "+key;
  button.addEventListener('click', initiateSetTileRequest);
  return button;
}

// ### Control Panel functions ###

function levelGenerate(){
  //Amend in case generation options become available
  postGenerateLevelRequest();
}

function aiStepStart(event){
  const button = event.target;
  button.dataset.intervalId = setInterval(postAiStepRequest, 1000).toString();
  button.onclick = aiStepStop;
  button.innerText = "Stop AI Step";
}

function aiStepStop(event){
  const button = event.target;
  clearInterval(parseInt(button.dataset.intervalId));
  button.onclick = aiStepStart;
  button.innerText = "Start AI Step";
}

function switchTrainingButton(isTraining){
  const toggleTrainingButton = document.getElementById('training-toggle-button');
  console.log(isTraining);
  if(isTraining === ""){
    toggleTrainingButton.onclick = postTrainingStartRequest;
    toggleTrainingButton.innerText = "Start Training";
  }
  else {
    toggleTrainingButton.onclick = postTrainingStopRequest;
    toggleTrainingButton.innerText = "Stop Training";
  }
}

// ### Set functions ###
function initiateActorActionRequest(event){
  const button = event.target;
  const actor = document.getElementById("actor-"+button._actorIndex);
  postActorAction(actor, button._actionId);
}

function initiateSetTileRequest(event){
  const button = event.target;
  const tileIndex = button._tileIndex;
  const condition = button._conditionKey;
  const tile = document.getElementById("tile-"+tileIndex);

  const newTileValue = tile._conditionsValue ^ tileFlags[condition];

  const payload = {
    x: tile._posX,
    y: tile._posY,
    value: newTileValue
  }

  postTileSetRequest(payload, tile);
}

// ### Net functions ###
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
          if(e instanceof SyntaxError) {
            console.log("Request returned Faulty JSON, attempting callback without additional parameters.");
            callback();
          }
          else {
            failure_callback(e);
          }
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
   load_backend_url('getLevel', res => {
      worldData = res;
      callback(worldData);
     },
    {}, handleError);
}

function fetchTrainingState(callback){
  load_backend_url('is_training', callback, {}, handleError)
}

function postActorAction(actor, actionId) {
  const updateFunction = showActorStats.bind(null, actor);
  const fetchAndUpdate = fetchAllData.bind(null, updateFunction);
  load_backend_url('manualAct', fetchAndUpdate,
    {actionId: actionId}, handleError);
}

function postTileSetRequest(payload, tile){
  const updateFunction = showTileStats.bind(null, tile);
  const fetchAndUpdate = fetchAllData.bind(null, updateFunction);
  load_backend_url('setTile', fetchAndUpdate,
    payload, handleError);
}

function postGenerateLevelRequest(){
  load_backend_url('level_generate', fetchAllData, {})
}

function postResetStatsRequest(){
  load_backend_url('reset_stats', fetchAllData, {}, handleError);
}

function postAiStepRequest(){
  load_backend_url('ai_step', fetchAllData, {}, handleError);
}

function postLoadBrainRequest(){
  load_backend_url('load_training_actor', fetchAllData, {}, handleError);
}

function postLoadTrainingLevelRequest(){
  load_backend_url('load_training_level', fetchAllData, {}, handleError);
}

function postTrainingStartRequest(){
  const callback = fetchTrainingState.bind(null, switchTrainingButton);
  load_backend_url('start_training', callback, {}, handleError);
}

function postTrainingStopRequest(){
  const callback = fetchTrainingState.bind(null, switchTrainingButton);
  load_backend_url('stop_training', callback, {}, handleError);
}
