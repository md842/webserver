import './Home.css';

export default function Home(){
	return(
		<>
      <main>
        <div className="profile">
          <section>
            <h1>Max Deng</h1>
            <img src="/picture.jpg"/>
            <h5>Computer Science B.S.</h5>
            <h5>University of California Los Angeles (UCLA)</h5>
            <a href="https://github.com/md842">
              <img src="/github-mark.svg" height="64px"/>
            </a>
          </section>

          <aside>
            <h3>About Me</h3>
            <p>
              Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum consequat id lacus in sodales. Aenean porttitor libero id arcu aliquet venenatis. Mauris interdum risus eget nisi maximus feugiat. Duis in vulputate eros, at dapibus diam. Ut in magna maximus, bibendum massa in, ornare mi. Integer sit amet convallis massa. Praesent pharetra ac sem sed egestas. Ut vitae dolor non dolor lacinia malesuada.
              <br/><br/>
              Donec vulputate auctor ipsum, in faucibus nibh commodo non. Maecenas placerat at nulla a sollicitudin. Suspendisse id massa dui. Proin at nisi vitae dolor vulputate gravida. Morbi feugiat tristique consequat. Vivamus metus lacus, cursus nec odio et, placerat feugiat purus. Nulla semper, odio ut tincidunt euismod, velit elit semper nisi, at lacinia quam ante nec mauris. Cras nec sem justo.
              <br/><br/>
              Nulla facilisi. Morbi commodo vulputate nibh. Pellentesque vel nisi vel ligula tincidunt pellentesque facilisis ut magna. Sed dolor tortor, sollicitudin ac justo eu, ultricies tempus nisi. Cras vel feugiat sapien, ac aliquet leo. Cras congue elit risus, a aliquet ipsum mollis sit amet. Ut vitae lorem lectus. Nullam hendrerit mi non risus malesuada aliquam. Maecenas pulvinar efficitur lobortis. Aenean non iaculis eros. Etiam molestie tortor sed tellus sollicitudin sollicitudin.
            </p>
          </aside>
        </div>
      </main>
		</>
	);
}