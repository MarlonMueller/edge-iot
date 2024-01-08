<script>
	import { io } from 'socket.io-client';

	const socket = io();

	socket.on('connect', () => {
		console.log('Connected with socket ID: ' + socket.id);
	});
	socket.on('eventFromServer', (message) => {
		console.log(message);
		console.log(socket);
	});
</script>

<div>mic</div>
