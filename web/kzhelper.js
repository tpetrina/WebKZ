function load_map(path) {
	$.ajax({
		url: path,
		error: function (jqXHR, textStatus, errorThrown) {
			alert("Cannot load map from '" + path + "', error: " + textStatus);
		},
		success: function (data) {
			add_map_to_scene(data);
		}
	});
}

function load_path(path) {
	$.ajax({
		url: path,
		error: function (jqXHR, textStatus, errorThrown) {
			alert("Cannot load path from '" + path + "', error: " + textStatus);
		},
		success: function (data) {
			add_path_to_scene(data);
		}
	});
}

var map_mesh, map_geometry;
var map_material;

var k = 100;

function addNormalLine(vertex, normal) {
	// lines for geometry2
	lineGeometry = new THREE.Geometry();
	lineGeometry.vertices.push(new THREE.Vector3(vertex[0], vertex[1], vertex[2]));
	lineGeometry.vertices.push(new THREE.Vector3(
		vertex[0] + normal[0] * k,
		vertex[1] + normal[1] * k,
		vertex[2] + normal[2] * k
		));
	line = new THREE.Line(lineGeometry, new THREE.LineBasicMaterial({ color: 0xff0000 }));
	scene.add(line);
}

function addLine(point0, point1, color) {
	// lines for geometry2
	lineGeometry = new THREE.Geometry();
	lineGeometry.vertices.push(new THREE.Vector3(point0[0], point0[1], point0[2]));
	lineGeometry.vertices.push(new THREE.Vector3(point1[0], point1[1], point1[2]));
	line = new THREE.Line(lineGeometry, new THREE.LineBasicMaterial({ color: color }));
	scene.add(line);
}

function addVLine(v1, v2, color) {
	// lines for geometry2
	lineGeometry = new THREE.Geometry();
	lineGeometry.vertices.push(new THREE.Vector3(v1.x, v1.y, v1.z));
	lineGeometry.vertices.push(new THREE.Vector3(v2.x, v2.y, v2.z));
	line = new THREE.Line(lineGeometry, new THREE.LineBasicMaterial({ color: color }));
	scene.add(line);
	return line;
}

var defaultTexture;
loadDefaultTexture();
function loadDefaultTexture() {
	defaultTexture = THREE.ImageUtils.loadTexture('dirt.jpg');
	defaultTexture.wrapS = THREE.RepeatWrapping;
	defaultTexture.wrapT = THREE.RepeatWrapping;
}

function TextureHolder(name) {
	this.name = name;
	this.glTexture = new THREE.ImageUtils.loadTexture(name);
	this.loaded = false;
	this.failed = false;
	this.glTexture.wrapS = THREE.RepeatWrapping;
	this.glTexture.wrapT = THREE.RepeatWrapping;
	this.glTexture.flipY = false;

	var that = this;
	this.glTexture.image.onload = function () {
		console.log("loaded texture " + that.name);
		that.loaded = true;
	};

	this.glTexture.image.onerror = function () {
		console.log("could not load " + that.name);
		that.loaded = true;
		that.failed = true;
	};
}

// keep all textures here
var textures = [];
var meshes = [];

function Mesh(texname) {
	this.texname = texname;
	this.geometry = new THREE.Geometry();
	this.geometry.faceVertexUvs = [[]];
}

Mesh.prototype.waitForTexture = function () {
	this.geometry.computeVertexNormals();
	this.geometry.computeBoundingSphere();

	var texture = defaultTexture;
	if (!textures.hasOwnProperty(this.texname)) {
		console.log("There is no such texture as " + this.texname);
		that.setDefaultMaterial();
		that.createMesh();
		scene.add(that.mesh);
	}
	else if (textures[this.texname].loaded) {
		if (textures[this.texname].failed) {
			console.log("Texture named '" + this.texname + "' could not be loaded, switching to default");
			that.setDefaultMaterial();
			that.createMesh();
			scene.add(that.mesh);
		} else {
			console.log("texture named '" + this.texname + "' loaded");
			that.setMaterial(textures[that.texname].glTexture);
			that.createMesh();
			scene.add(that.mesh);
		}
	}
	else {
		var that = this;
		textures[this.texname].glTexture.image.onload = function () {
			console.log("texture " + that.texname + " loaded");
			that.setMaterial(textures[that.texname].glTexture);
			that.createMesh();
			scene.add(that.mesh);
		}
		textures[this.texname].glTexture.image.onerror = function () {
			console.log("texture " + that.texname + " failed to load");
			that.setDefaultMaterial();
			that.createMesh();
			scene.add(that.mesh);
		}
	}
}

Mesh.prototype.setDefaultMaterial = function () {
	this.material = new THREE.MeshLambertMaterial({ color: 0xffffff, wireframe: false, map: defaultTexture });
}
Mesh.prototype.setMaterial = function (texture) {
	this.material = new THREE.MeshLambertMaterial({ color: 0xffffff, wireframe: false, map: texture });
}
Mesh.prototype.createMesh = function (texture) {
	this.mesh = new THREE.Mesh(this.geometry, this.material);
}

function add_map_to_scene(map) {
    if (meshes != null && meshes != undefined)
    {
        for (var index in meshes)
            scene.remove(meshes[index].mesh);
    }

    textures = [];
	meshes = [];
	vertex_assoc = [];

	// go through each face
	for (var i in map.faces) {
		var face = map.faces[i];

		var mesh;
		if (meshes.hasOwnProperty(face.texture)) {
			mesh = meshes[face.texture];
		}
		else {
			meshes[face.texture] = (mesh = new Mesh(face.texture));
			textures[face.texture] = new TextureHolder("textures/" + face.texture + ".jpg");
			vertex_assoc[face.texture] = [];
		}

		var vertices = vertex_assoc[face.texture];
		var faceVertices = [];
		for (var j in face.vertices) {
			var vi = face.vertices[j];
			if (!vertices.hasOwnProperty(vi)) {
				// add vertex to geometry and map
				mesh.geometry.vertices.push(new THREE.Vector3(map.vertices[vi][0], map.vertices[vi][1], map.vertices[vi][2]));
				vertices[vi] = mesh.geometry.vertices.length - 1;
			}

			faceVertices.push(vertices[vi]);
		}

		mesh.geometry.faces.push(new THREE.Face3(faceVertices[0], faceVertices[1], faceVertices[2]));
		mesh.geometry.faceVertexUvs[0].push([
				new THREE.UV(face.faceUVs[0], face.faceUVs[1]),
				new THREE.UV(face.faceUVs[2], face.faceUVs[3]),
				new THREE.UV(face.faceUVs[4], face.faceUVs[5])
		]);
	}

	for (var i in meshes) {
		meshes[i].waitForTexture();
	}
}

var pathFrames = null;
var currentFrame = 0;
var frameLine;
var totalLength;
var currentTime = 0;

var line;

function add_path_to_scene(path) {
	pathFrames = path.frames;
	// lines for geometry2
	lineGeometry = new THREE.Geometry();
	var frameOffset = 0;
	for (var frameIndex in path.frames) {
		var frame = path.frames[frameIndex];
		frame.frameOffset = frameOffset;
		frame.length = frame.length * 1000; // TODO: remove
		frameOffset += frame.length;
		lineGeometry.vertices.push(new THREE.Vector3(frame.position[0], frame.position[1], frame.position[2]));
	}
	totalLength = frameOffset;

	line = new THREE.Line(lineGeometry, new THREE.LineBasicMaterial({ color: 0x0000ff }));
	// scene.add(line);

	frameLine = lineForFrame(0);
}

function lineForFrame(frameIndex) {
	var k = 1;
	var vPos = new THREE.Vector3(
		pathFrames[frameIndex].position[0],
		pathFrames[frameIndex].position[1] + 25,
		pathFrames[frameIndex].position[2]);
	var vLookAt = new THREE.Vector3(
		pathFrames[frameIndex].position[0] + pathFrames[frameIndex].orientation[0] * k,
		pathFrames[frameIndex].position[1] + pathFrames[frameIndex].orientation[1] * k,
		pathFrames[frameIndex].position[2] + pathFrames[frameIndex].orientation[2] * k
		);
	var vLookAt2 = new THREE.Vector3(
		pathFrames[frameIndex].orientation[0] * k,
		pathFrames[frameIndex].orientation[1] * k,
		pathFrames[frameIndex].orientation[2] * k
		);

	var vLookAt3 = new THREE.Vector3();
	vLookAt3.copy(vPos);
	vLookAt3.add(vLookAt3, new THREE.Vector3(
		-Math.sin((pathFrames[frameIndex].orientation[2] - 90) / 57.295800000006835) * 100,
		Math.sin((pathFrames[frameIndex].orientation[1] - 90) / 57.295800000006835) * 100,
		Math.cos((pathFrames[frameIndex].orientation[2] - 90) / 57.295800000006835) * 100));

	// position camera to the first frame
	camera.position = vPos;
	camera.lookAt(vLookAt3);

	//console.log(frameIndex);
	//console.log(vPos);
	//console.log(vLookAt);

	return addVLine(new THREE.Vector3(0, 0, 0), vLookAt2, 0xff00ff);
}

function add_map_to_scene2(map) {
	map_geometry = new THREE.Geometry();

	for (var i = 0; i < map.vertices.length; i++) {
		var vertex = map.vertices[i];
		var x = vertex[0];
		var y = vertex[2];
		var z = vertex[1];
		map_geometry.vertices.push(new THREE.Vector3(x, y, z));
	}

	map_geometry.faceVertexUvs = [[]];

	for (var i = 0; i < map.faces.length; ++i) {
		var face = map.faces[i];
		map_geometry.faces.push(new THREE.Face3(face.vertices[0], face.vertices[1], face.vertices[2]));
		map_geometry.faceVertexUvs[0].push([
			new THREE.UV(face.faceUVs[0], face.faceUVs[1]),
			new THREE.UV(face.faceUVs[2], face.faceUVs[3]),
			new THREE.UV(face.faceUVs[4], face.faceUVs[5])
		]);

		if (false) {
			addNormalLine([
				map.vertices[face.vertices[0] * 3],
				map.vertices[face.vertices[0] * 3 + 2],
				map.vertices[face.vertices[0] * 3 + 1]], face.normal);

			addNormalLine([
				map.vertices[face.vertices[1] * 3],
				map.vertices[face.vertices[1] * 3 + 2],
				map.vertices[face.vertices[1] * 3 + 1]], face.normal);

			addNormalLine([
				map.vertices[face.vertices[2] * 3],
				map.vertices[face.vertices[2] * 3 + 2],
				map.vertices[face.vertices[2] * 3 + 1]], face.normal);
		}
	}

	map_geometry.computeVertexNormals();
	map_geometry.computeBoundingSphere();

	map_material = new THREE.MeshLambertMaterial({ color: 0xffffff, wireframe: false, map: defaultTexture });
	map_mesh = new THREE.Mesh(map_geometry, map_material);

	scene.add(map_mesh);
	console.log("level loaded successfully");
}